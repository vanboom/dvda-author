/*
  
ats2wav - extract uncompressed LPCM audio from a DVD-Audio disc

Copyright Dave Chapman <dave@dchapman.com> 2005,
revised by Fabrice Nicol <fabnicol@users.sourceforge.net> 2008
for Windows compatibility, directory output and dvda-author integration.

The latest version can be found at http://dvd-audio.sourceforge.net

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


#include "ats2wav.h"
#include "ats.h"
#include "c_utils.h"

#include "winport.h"
#include "multichannel.h"
#include "auxiliary.h"


extern globalData globals;
extern uint8_t channels[21];

unsigned char wav_header[80]= {'R','I','F','F',   //  0 - ChunkID
                               0,0,0,0,            //  4 - ChunkSize (filesize-8)
                               'W','A','V','E',    //  8 - Format
                               'f','m','t',' ',    // 12 - SubChunkID
                               40,0,0,0,           // 16 - SubChunk1ID  // 40 for WAVE_FORMAT_EXTENSIBLE
                               0xFE, 0xFF,         // 20 - AudioFormat (1=16-bit)
                               2,0,                // 22 - NumChannels
                               0,0,0,0,            // 24 - SampleRate in Hz
                               0,0,0,0,            // 28 - Byte Rate (SampleRate*NumChannels*(BitsPerSample/8)
                               4,0,                // 32 - BlockAlign (== NumChannels * BitsPerSample/8)
                               0x10,0,             // 34 - BitsPerSample (16)
                               0x16,0,             // 36 - cbSize Size of extension (22)
                               0x10,0,             // 38 - wValidBitsPerSample (16 bits)
                               0,0,0,0,            // 40 - dwChannelMask
                               0x01,0x00,          // 44 - subFormat 0x1 for PCM
                               0,0,0,0,0x10,0,0x80,0,0,0xaa,0,0x38,0x9b,0x71,                    // 46 - GUID (fixed string)
                               'f','a','c','t',    // 60 - "fact"
                               4,0,0,0,            // 64 - ckSize (4)
                               0,0,0,0,            // 68 - dwSampleLength = NumChannels * ckSize/(BitsPerSample/8)
                               'd','a','t','a',    // 72 - ckID
                               0,0,0,0             // 76 - ckSize -> // 79
                              };

// Reverse table (to be used to convert AOBs to WAVs

static const uint8_t  T[2][6][36]=
     {{ {0}, // 4
        {0}, // 8
        {5, 4, 7, 6, 1, 0, 9, 8, 11, 10, 3, 2}, 
        {9, 8, 11, 10, 1, 0, 3, 2, 13, 12, 15, 14, 5, 4 ,7, 6}, 
        {9, 8, 11, 10, 13, 12, 1, 0, 3, 2, 15, 14, 17, 16, 19, 18, 5, 4, 7, 6}, //20, rev
        {13, 12, 15, 14, 17, 16, 1, 0, 3, 2, 5, 4, 19, 18, 21, 20, 23, 22, 7, 6, 9, 8, 11, 10}}, //rev
      {{4,  1,  0,  5,  3,  2},
        {8, 1, 0, 9, 3, 2, 10, 5, 4, 11, 7, 6},
        {14, 7, 6, 15, 9, 8, 4, 1, 0, 16, 11, 10, 17, 13, 12, 5, 3, 2},
        {20, 13, 12, 21, 15, 14, 8, 1, 0, 9, 3, 2, 22, 17, 16, 23, 19, 18, 10, 5, 4, 11, 7, 6},
        {24, 13, 12, 25, 15, 14, 26, 17, 16, 8, 1, 0, 9, 3,  2,  27, 19, 18, 28, 21, 20, 29,  23, 22,  10, 5, 4, 11, 7, 6}, //30, rev
        {28, 13, 12, 29, 15, 14,  8, 1, 0,  9, 3, 2, 30, 17, 16, 31, 19, 18, 32, 21, 20, 33, 23, 22, 10, 5, 4, 11, 7,  6, 34, 25, 24, 35, 27, 26 }}
    };

// sizes of preceding table

static const uint8_t  permut_size[2][6] = {{4, 8, 12, 16, 20, 24}, {6, 12, 18, 24, 30, 36}};

static void deinterleave_24_bit_sample_extended(uint8_t channels, int count, uint8_t *buf)
{
    // Processing 16-bit case
    int i, size=channels*6;
    // Requires C99
    uint8_t _buf[size];

    for (i=0; i < count ; i += size)
        permutation(buf+i, _buf, 1, channels, T, size);

}

static void deinterleave_sample_extended(uint8_t channels, int count, uint8_t *buf)
{

    int x,i, size = channels * 4;

    uint8_t _buf[size];

    switch (channels)
    {
    case 1:
    case 2:
        for (i = 0; i < count; i += 2)
        {
            x = buf[i];
            buf[i] = buf[i + 1];
            buf[i + 1] = x;
        }
        break;

    default:
        for (i =0; i < count ; i += size)
            permutation(buf+i, _buf, 0, channels, T, size);
    }
}

static void convert_buffer(WaveHeader* header, uint8_t *buf, int count)
{
    switch (header->wBitsPerSample)
    {
        case 24:

            deinterleave_24_bit_sample_extended(header->channels, count, buf);
            break;

        case 16:
            deinterleave_sample_extended(header->channels, count, buf);
            break;

        default:
            // FIX: Handle 20-bit audio and maybe convert other formats.
            printf("[ERR]  %d bit %d channel audio is not supported\n", header->wBitsPerSample, header->channels);
            return;
            //exit(EXIT_FAILURE);
    }
}


inline static void  aob_open(WaveData *info)
{

    if (file_exists(info->infile.filename))
        info->infile.filesize = stat_file_size(info->infile.filename);

    info->infile.fp = fopen(info->infile.filename, "rb")  ;

    if (info->infile.fp != NULL)
    {
        info->infile.isopen = true;
    }
    else
    {
        foutput("File: %s\n", info->infile.filename);
        EXIT_ON_RUNTIME_ERROR_VERBOSE("INFILE open issue.")
    }
}

inline static void wav_output_path_create(const char* dirpath, WaveData *info)
{
    static int title;

    char Title[14] = {0};
    sprintf(Title, "title_%d.wav", ++title);

    info->outfile.filename = filepath(dirpath, Title);
}

inline static void wav_output_open(WaveData *info)
{
    if (globals.veryverbose)
    {
        foutput(INF "Opening file %s ...\n", info->outfile.filename);
    }
    
    info->outfile.fp = fopen(info->outfile.filename, "ab");
    if (info->outfile.fp != NULL)
    {
        info->outfile.isopen = true;
    }
    else
    {
        EXIT_ON_RUNTIME_ERROR_VERBOSE("OUTFILE open issue.")
    }
}


inline static int calc_position(FILE* fileptr, const uint64_t offset0)
{
    uint8_t buf[4];
    int position;

    fseeko(fileptr, offset0 + 14, SEEK_SET);
    int result = fread(buf, 4, 1, fileptr);
    
    if (result != 1)
    {
        EXIT_ON_RUNTIME_ERROR_VERBOSE(ERR " Error detecting packet position.")
    }
    

    /* got to system header and read first 4 bytes to detect whether pack is start or not */

    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1 && buf[3] == 0xBB)
    {
        position = FIRST_PACK;
    }
    else
    {
        /* go to end of sector : if end of file, then last pack, idem if new pack detected ; otherwise middle pack */

        int res = fseeko(fileptr, 2044, SEEK_CUR);

        if (res != 0)
        {
            position = LAST_PACK;
        }
        else
        {
            int n = fread(buf, 1, 4, fileptr);

            if (n != 4 || (buf[0] == 0 && buf[1] == 0 && buf[2] == 1 && buf[3] == 0xBB))
            {
                position = LAST_PACK;
            }
            else
            {
                position = MIDDLE_PACK;
            }
        }
    }
    
    return(position);
}

inline static int peek_pes_packet_audio(WaveData *info, WaveHeader* header, _Bool *status)
{
    if (! info->infile.isopen) aob_open(info);
    
    uint64_t offset0 = ftello(info->infile.fp);
     
    int position = calc_position(info->infile.fp, offset0);
    
    fseeko(info->infile.fp, offset0 + 35 + (position == FIRST_PACK ? 21 : 0), SEEK_SET);

    uint8_t sample_size[1] = {0};
    uint8_t sample_rate[1] = {0};
    
    int result = fread(sample_size, 1, 1, info->infile.fp);
    
    if (result != 1)
    {
        EXIT_ON_RUNTIME_ERROR_VERBOSE(ERR " Error detecting sample size.")
    }
    
    result = fread(sample_rate, 1, 1, info->infile.fp);
    if (result != 1)
    {
        EXIT_ON_RUNTIME_ERROR_VERBOSE(ERR " Error detecting sample rate.")
    }
    
    uint8_t high_nibble = (sample_rate[0] & 0xf0) >> 4;

    switch(high_nibble)
    {
        case 0:
            header->dwSamplesPerSec = 48000;
            break;
        case 0x1:
            header->dwSamplesPerSec = 96000;
            break;
        case 0x2:
            header->dwSamplesPerSec = 192000;
            break;
        case 0x8:
            header->dwSamplesPerSec = 44100;
            break;
        case 0x9:
            header->dwSamplesPerSec = 88200;
            break;
        case 0xa:
            header->dwSamplesPerSec = 176400;
            break;
        default:
            *status = INVALID;
            break;
    }

    if (sample_size[0] == 0x0f || sample_size[0] == 0x2f)
    {
        if ((sample_rate[0] & 0xf) != 0xf)
        {
            foutput("%s", "[ERR]  Coherence_test (peek) : sample_rate and sample_size are incoherent (no 0xf lower nibble).\n");
            fflush(NULL);
            *status = INVALID;
        }
    }
    else
    if (sample_size[0] == 0x00 || sample_size[0] == 0x22)
    {
        if ((sample_rate[0] & 0xf) != high_nibble)
        {
            foutput("%s", "[ERR]  Coherence_test (peek) : sample_rate and sample_size are incoherent (lower nibble != higher nibble).\n");
            *status = INVALID;
            fflush(NULL);
        }
    }

    header->wBitsPerSample = (sample_size[0] == 0x2f || sample_size[0] == 0x22) ? 24 : ((sample_size[0] == 0x0f || sample_size[0] == 0x00) ? 16 : 0) ;

    if (! header->wBitsPerSample) *status = INVALID;

    fseeko(info->infile.fp, 1, SEEK_CUR);

    uint8_t channel_assignment[1] = {0};

    result = fread(channel_assignment, 1, 1, info->infile.fp);

    if (result != 1)
    {
        EXIT_ON_RUNTIME_ERROR_VERBOSE(ERR " Error detecting channel.")
    }
    
    if (channel_assignment[0] > 20) *status = INVALID;

    header->channels = (channel_assignment[0] < 21) ? channels[channel_assignment[0]] : 0;

    header->nBlockAlign =  header->wBitsPerSample / 8 * header->channels ;
    header->nAvgBytesPerSec = header->nBlockAlign * header->dwSamplesPerSec;

    uint32_t cga2wav_channels[21] = {0x4, 0x3,   0x103, 0x33,  0xB,  0x10B, 0x3B,  0x7,  0x107, 0x37,
                                     0xF, 0x10F, 0x3F,  0x107, 0x37, 0xF,   0x10F, 0x3F, 0x3B,  0x37, 0x3B};

    if (channel_assignment[0] < 21)
    {
      header->dwChannelMask = cga2wav_channels[channel_assignment[0]];
    }

   
    fseeko(info->infile.fp, offset0, SEEK_SET);

    return position;
}


inline static int get_pes_packet_audio(WaveData *info, WaveHeader *header, uint64_t numbytes, int* remainder)
{
    int position;
    static uint64_t fpout_size;
    int audio_bytes;
    uint8_t PES_packet_len_bytes[2];
    uint8_t audio_buf[2048 * 2]; // in case of incomplete buffer shift

    // CAUTION : check coherence of this table and the S table of audio.c, of which it is only a subset.

    const uint16_t T[2][6][6]=     // 16-bit table
    {
         {{ 	2000, 16,   22, 11, 16, 16},
            {	2000, 16,   28, 11, 16, 10},
            { 	2004, 24,   24, 15, 12, 10},
            { 	2000, 16,   28, 11, 16, 10},
            { 	2000, 20,   22, 15, 16, 10},
            { 	1992, 24,   22, 10, 10, 10}},
        // 24-bit table
        {{    	2004, 24,   22, 15, 12, 12},
            { 	2004, 24,   24, 15, 12, 10},
            { 	1998, 18,   28, 15, 10, 10},
            { 	1992, 24,   22, 10, 10, 10},
            { 	1980,  0,   22, 15, 10, 10},
            { 	1980,  0,   22, 15, 16, 16}}
    };

    const short int table_index = header->wBitsPerSample == 24 ? 1 : 0;

#   define X T[table_index][header->channels-1]

    const int lpcm_payload = X[0];
    const int firstpackdecrement = X[1];
    const int lastpack_audiopesheaderquantity = X[2];
    const int firstpack_lpcm_headerquantity = X[3];
    const int midpack_lpcm_headerquantity   = X[4];
    const int lastpack_lpcm_headerquantity  = X[5];

#   undef X

    //////////////////////////////

    if (! info->infile.isopen) aob_open(info);

    uint64_t offset0 = ftello(info->infile.fp);
    
    int res  = 0, result;    
    
    do {
            if (*remainder)
            {
                if (globals.veryverbose)
                {
                    foutput(MSG_TAG "%s", "Cutting AOB audio for gapless track extraction.\n");
                }
                
                position = CUT_PACK;
            }
            else
            {
               position = calc_position(info->infile.fp, offset0);
            }
            
            switch(position)
            {
                case FIRST_PACK :
                    audio_bytes = lpcm_payload - firstpackdecrement;
                    fseeko(info->infile.fp, offset0 + 53 + firstpack_lpcm_headerquantity, SEEK_SET);
                    break;
        
                case MIDDLE_PACK :
                    audio_bytes = lpcm_payload;
                    fseeko(info->infile.fp, offset0 + 32 + midpack_lpcm_headerquantity, SEEK_SET);
                    break;
        
                case LAST_PACK :
                    fseeko(info->infile.fp, offset0 + 18, SEEK_SET);
                    result = fread(PES_packet_len_bytes, 1, 2, info->infile.fp);
                    if (result != 2)
                    {
                            EXIT_ON_RUNTIME_ERROR_VERBOSE(ERR " Error detecting last pack.")
                    }
                    audio_bytes = (PES_packet_len_bytes[0] << 8 | PES_packet_len_bytes[1]) - lastpack_audiopesheaderquantity;
                    /* skipping rest of audio_pes_header, i.e 8 bytes + lpcm_header */
                    fseeko(info->infile.fp, offset0 + 32 + lastpack_lpcm_headerquantity, SEEK_SET);
                    break;
                    
                case CUT_PACK :
                    res = fread(audio_buf, 1, *remainder, info->infile.fp);
                    *remainder -= res;  // normally 0
                    if (*remainder && globals.veryverbose) 
                        foutput("%s%luX\n", 
                                WAR "Incomplete remainder detected at offset ",
                                ftello(info->infile.fp));
                    break;
            }
    } 
    while(position == CUT_PACK);

    // Here "cut" for a so-called 'gapless' track
    // The cut should respect the size of permutation tables.
    
    if (numbytes && fpout_size + audio_bytes > numbytes)
    {
        *remainder = fpout_size + audio_bytes - numbytes;
        audio_bytes  = numbytes - fpout_size;
        
        if (globals.veryverbose)
        {
            foutput(MSG_TAG "%s, offset: %lu, remainder: %d bytes\n", 
                    "Cutting AOB audio for track extraction at end of track.",
                    numbytes,
                    *remainder);
        }
        
        position = CUT_PACK;
    }
    
    res += fread(audio_buf + res, 1, audio_bytes, info->infile.fp);

    if (globals.maxverbose)    
    {
       foutput(MSG_TAG "Audio bytes: %d, res: %d\n", audio_bytes, res);
       if (res != audio_bytes)
           foutput(WAR "Caution : Read %d instead of %d\n", res, audio_bytes);
    }
    
    convert_buffer(header, audio_buf, res);

    fpout_size += fwrite(audio_buf, 1, res, info->outfile.fp);

    
    if (globals.maxverbose)
           foutput(MSG_TAG "File size : %lu\n", fpout_size);
    
    if (position == LAST_PACK || position == CUT_PACK)
    {
        fpout_size += header->header_size_out;

        fseeko(info->outfile.fp, 0, SEEK_END);

        if (ftello(info->outfile.fp) != (off64_t) fpout_size)
        {
            foutput(ERR  "Audio decoding outfile mismatch. Decoded %" 
                    PRIu64 
                    " bytes yet file size audio is %" 
                    PRIu64 
                    " bytes.\n", 
                    fpout_size,
                    ftello(info->outfile.fp));
        }

        if (globals.veryverbose)
            foutput(INF "%s", "Cutting gapless track...\n");
                    
        foutput(INF "Writing %s (%.2f MB)...\n",
                filename(info->outfile),
                (double) fpout_size / (double) (1024 * 1024));
        
        S_CLOSE(info->outfile)
        info->outfile.filesize = fpout_size;
        fpout_size = 0;
    }

    uint64_t offset1 = offset0 + (position == CUT_PACK ? 0 : 2048);
        
    if (offset1 >= filesize(info->infile))
        position = END_OF_AOB;
    else
        fseeko(info->infile.fp, offset1, SEEK_SET);
   
    if (globals.maxverbose)
           foutput(MSG_TAG "Position : %d\n", position);
    
    return(position);
}

int get_ats_audio_i(int i, fileinfo_t files[9][99], WaveData *info)
{
    uint64_t pack = 0;
    int j = 0; // necessary (track count)
    int pack_rank = FIRST_PACK;

    WaveHeader header;
    
    int remainder = 0;
    const char* dirpath = info->outfile.filename;
    
    while (pack_rank != END_OF_AOB)
    {
        /* First pass to get basic audio characteristics (sample rate, bit rate, cga */
        
        _Bool status = VALID;

        errno = 0;
        if (globals.veryverbose)
            foutput("%s %d %s %d\n", MSG_TAG "Group ", i + 1, "Track ", j + 1);
        do
        {
            pack_rank = peek_pes_packet_audio(info, &header, &status);
            
            if (status == VALID) break;
        }
        while (pack_rank == FIRST_PACK || pack_rank == MIDDLE_PACK);
        
        files[i][j].bitspersample = header.wBitsPerSample;
        files[i][j].samplerate = header.dwSamplesPerSec;
        files[i][j].channels = header.channels;
        
        uint64_t x = 0, numsamples, numbytes = 0;
        
        if (files[i][j].PTS_length)     // Use IFO files
        {
            numsamples = (files[i][j].PTS_length * files[i][j].samplerate) / 90000;
            
            if (numsamples)
                x = 90000 * numsamples;
            else
            {
                foutput("%s", ERR "Found null samplerate or PTS length. Exiting...\n");
                clean_exit(EXIT_FAILURE);
            }
            
            // Adjust for rounding errors:
            
            if (x < files[i][j].PTS_length * files[i][j].samplerate)
            {
                ++numsamples;
            }
            
            numbytes = (numsamples * files[i][j].channels * files[i][j].bitspersample) / 8;
            const short int table_index = header.wBitsPerSample == 24 ? 1 : 0;
            int modulo = permut_size[table_index][header.channels - 1];
            
            // Taking modulo
            
            numbytes -= numbytes % modulo;
        }

//        if (errno)
//        {
//           foutput(ERR "Error while trying to recover audio characteristics of file %s.\n        Exiting...\n",
//                   filename(info->infile));
//           exit(-7);
//        }

//        if (status == INVALID)
//        {
//           foutput(ERR "Error while trying to recover audio characteristics : invalid status or input file %s.\n        Exiting...\n",
//                   filename(info->infile));
//           exit(-8);
//        }

        wav_output_path_create(dirpath, info);
        files[i][j].filename = info->outfile.filename;
        wav_output_open(info);

        _Bool debug;

        if (globals.fixwav_prepend)
        {
            /* generate header in empty file. We must allow prepend and in_place for empty files */

            debug = globals.debugging;

            /* Hush it up as there will be spurious error mmsg */
            globals.debugging = false;

            info->outfile.filesize = 0;  // necessary to reset so that header can be generated in empty file
            filestat_t temp = info->infile;
            
            info->infile = info->outfile;
            info->prepend = true;
            info->infile.fp = fopen(info->infile.filename, "wb+");
            
            if (info->infile.fp != NULL)
            {
                info->infile.isopen = true;
            }
            else
            {
                EXIT_ON_RUNTIME_ERROR_VERBOSE("WAV header-prepending issue.")
            }
            
            fixwav(info, &header);

            S_CLOSE(info->outfile)  // necessary here, forbidden with the second fixwav below.

            info->infile = temp;
            errno = 0;

            if (globals.veryverbose)
                foutput(MSG_TAG "%s\n", "Header prepended.");
            
            wav_output_open(info);
        }

        /* second pass to get the audio */

        do
        {
            if (globals.maxverbose)
                foutput(MSG_TAG "Pack %lu, Numbytes %lu, remainder %d\n", pack + 1, numbytes, remainder);
                        
            pack_rank = get_pes_packet_audio(info, &header, numbytes, &remainder);
            ++pack;
            
        }
        while (pack_rank == FIRST_PACK || pack_rank == MIDDLE_PACK);

        foutput(MSG_TAG "Read %" PRIu64 " PES packets.\n", pack);

        if (globals.fixwav_prepend)
        {
            /* WAV output is now OK except for the wav file size-based header data.
             * ckSize, data_ckSize and nBlockAlign must be readjusted by computing
             * the exact audio content bytesize. Also we no longer prepend the header
             * but overwrite the existing one */

            info->prepend = false;
            filestat_t temp = info->infile;
            info->infile = info->outfile;

            fixwav(info, &header);

            if (j == 99)
            {
                EXIT_ON_RUNTIME_ERROR_VERBOSE("DVD-Audio specifications only allow 99 titles per group.")
            }

            // Possible adjustment here
            
            files[i][j].numbytes = header.data_cksize;

            globals.debugging = debug;
            info->infile = temp;
            errno = 0;
        }

        S_CLOSE(info->outfile)
                
        if (pack_rank == LAST_PACK || pack_rank == CUT_PACK)
        {
            if (globals.veryverbose)        
                    foutput("%s\n", INF "Closing track and opening new one.");
        }
        else
        if (pack_rank == END_OF_AOB)
        {
            if (globals.veryverbose)        
                    foutput("%s\n", INF "Closing last track of AOB.");
        }
        
        ++j;  // track rank
        
    }

    S_CLOSE(info->infile)
    free(info);    
    
    return(errno);
}

static void audio_extraction_layout(fileinfo_t files[9][99])
{
    foutput("\n%s", "DVD Layout\n");
    foutput("%s\n",ANSI_COLOR_BLUE"Group"ANSI_COLOR_GREEN"  Track    "ANSI_COLOR_YELLOW"Rate"ANSI_COLOR_RED" Bits"ANSI_COLOR_RESET"  Ch    Audio bytes  Filename\n");

    for (int i = 0; i < 9; ++i)
        for (int j = 0; j < 99 && files[i][j].filename != NULL; ++j)
        {
           foutput("  "ANSI_COLOR_BLUE"%d     "ANSI_COLOR_GREEN"%02d"ANSI_COLOR_YELLOW"  %6"PRIu32"   "ANSI_COLOR_RED"%02d"ANSI_COLOR_RESET"   %d   %10"PRIu64"   ",
                     i+1, j+1, files[i][j].samplerate, files[i][j].bitspersample, files[i][j].channels, files[i][j].numbytes);
           foutput("%s\n",files[i][j].filename);
        }

    foutput("\n\n%s\n\n", MSG_TAG "The Audio bytes column gives the exact audio size of extracted files,\n" MSG_TAG "excluding the header (44 bytes for mono/stereo or 80 bytes for multichannel.)");
}


int get_ats_audio()
{
    fileinfo_t files[9][99] = {{{0}}};
    
    for (int i = 0; i < 9 && globals.aobpath[i] != NULL; ++i)
    {
      if (globals.veryverbose)
         foutput("%s%d%s\n", INF "Extracting audio for AOB n°", i+1, " (1-based).");

      WaveData *info = NULL;
      
      get_ats_audio_i(i, files, info);
      
      if (globals.veryverbose)
              foutput("%s\n", INF "Reached ead of AOB.");
    }
    
    if (globals.fixwav_prepend)
        audio_extraction_layout(files);

    return(errno);
}


int scan_ats_ifo(fileinfo_t *files, uint8_t *buf)
{
    int i, j, k, t=0, ntracks, ntracks1, numtitles;
    i = 2048;
    numtitles = uint16_read(buf + i);
    
    uint8_t titleptr[numtitles];
    
    i += 8;
    ntracks = 0;
    
    for (j = 0; j < numtitles; ++j)
    {
        i += 4;
        titleptr[j] = uint32_read(buf + i);
        i += 4;
    }
    
    for (j = 0; j < numtitles; ++j)
    {
        i = 0x802 + titleptr[j];
        ntracks1 = buf[i];
        i += 14;
        
        t = ntracks;
        
        for (k = 0; k < ntracks1; ++k)
        {
            i += 10;
            
            files[t].PTS_length = uint32_read(buf + i);
            
            i += 10;
            ++t;
        }
        
        t = ntracks;
        
        /* 12 byte sector records */
        
        for (k = 0; k < ntracks1; ++k)
        {
            i += 4;
            files[t].first_sector = uint32_read(buf + i);
            
            i += 4;
            files[t].last_sector = uint32_read(buf + i);
            
            i += 4;
            ++t;
        }
        
        ntracks += ntracks1;
    }
    
    if (globals.debugging)
        for (i = 0; i < ntracks; ++i)
        {
            printf("     Track/N first sector  last sector   pts length\n     %02d/%02d    %12u %12u %12"PRIu64"\n\n",
                   i+1, ntracks, files[i].first_sector, files[i].last_sector, files[i].PTS_length);
        }
 
    return(ntracks);
}

int ats2wav(short ngroups_scan, const char* audiots_dir, const char *outdir, const extractlist* extract)
{
    FILE* file = NULL;
    unsigned int ntracks = 0;
    fileinfo_t files[9][99];
    memset(files, 0, sizeof(files));
    uint8_t buf[BUFFER_SIZE];
    uint16_t nbytesread=0; 
    
    /* First check the DVDAUDIO-ATS tag at start of ATS_XX_0.IFO */
        
    char filename[13] = {0};

    if (ngroups_scan > 9 || ngroups_scan < 1)
    {
        EXIT_ON_RUNTIME_ERROR_VERBOSE(ERR "Group rank should be between 1 and 9")
    }
    
    sprintf(filename, "ATS_0%d_0.IFO", ngroups_scan);
    
    if ((file = fopen(filename, "rb")) == NULL)
    {
        EXIT_ON_RUNTIME_ERROR_VERBOSE("IFO file could not be opened.")
    }
    
    if (globals.debugging)
        printf( INF "Reading file %s\n", filename);
    
    nbytesread = fread(buf, 1, sizeof(buf), file);
    
    if (globals.debugging)
        printf( INF "Read %d bytes\n", nbytesread);
    
    fclose(file);
    
    if (memcmp(buf, "DVDAUDIO-ATS", 12) != 0)
    {
        printf(ERR "%s is not an ATSI file (ATS_XX_0.IFO)\n", filename);
        return(EXIT_FAILURE);
    }
    
    printf("%c", '\n');
        
    /* now scan tracks to be extracted */
        
    ntracks = scan_ats_ifo(&files[0][0], buf);
    
    if (outdir == NULL) exit(0);

    if (globals.maxverbose)
    {
        EXPLAIN("%s%d%s\n", DBG " Scanning ", ntracks, "tracks")
    }
    
    if (! extract || (extract && extract->extracttitleset[ngroups_scan] == 1))
    {
       if (globals.veryverbose)
       {
          foutput("%s%d%s\n",
                  INF "Extracting audio for AOB n°",
                  ngroups_scan,
                  " (1-based).");
       }
 
       sprintf(filename, "ATS_0%d_1.AOB", ngroups_scan);
       
       WaveData *info = (WaveData*) calloc(1, sizeof(WaveData));

       info->database = NULL;
       info->filetitle = NULL;
       info->automatic = true;
       info->prepend = globals.fixwav_prepend;
       info->in_place = true;
       info->cautious = false;
       info->interactive = false;
       info->padding = false;
       info->prune = false;
       info->virtual = false;
       info->repair = 0;
       info->padbytes = 0;
       info->prunedbytes = 0;
       //info->infile =  filestat(false, 0, globals.aobpath[i], NULL);

       info->infile =  filestat(false,
                                0,
                                filepath(audiots_dir, filename),
                                NULL);

       info->outfile = filestat(false,
                                0,
                                outdir,
                                NULL);

       get_ats_audio_i(ngroups_scan - 1,
                       files,
                       info);
          
       if (globals.veryverbose)
       {
           foutput("%s\n",
                   INF "Reached ead of AOB.");
       }
    }
    
    audio_extraction_layout(files);
    
    return(EXIT_SUCCESS);
}