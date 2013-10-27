/*

File:    ats.c
Purpose: Create an Audio Titleset

dvda-author  - Author a DVD-Audio DVD

(C) Dave Chapman <dave@dchapman.com> 2005

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "structures.h"
#include "audio2.h"
#include "c_utils.h"
#include "auxiliary.h"
#include "commonvars.h"
#include "ats.h"


extern globalData globals;

uint8_t pack_start_code[4]= {0x00,0x00,0x01,0xBA};
uint8_t system_header_start_code[4]= {0x00,0x00,0x01,0xBB};
uint8_t program_mux_rate_bytes[3]= {0x01,0x89,0xc3};
uint64_t fileoffset;




inline   void  write_search_sequence(uint8_t* sequence, size_t sizeofsequence , FILE* filepointer, const char* ioflag)
{
    if (globals.veryverbose)
        hexdump_pointer(sequence, sizeofsequence);

    uint64_t offset_save=fileoffset;



    if (ioflag[0] == 'w')
    {
        if (!globals.nooutput)
            fwrite (sequence,sizeofsequence,1,filepointer);


        if (globals.veryverbose)
            EXPLAIN("%s%llu", "Wrote sequence at offset: ", fileoffset);

        fileoffset+=  sizeofsequence;

    }
    else
    {
        fileoffset=parse_file_for_sequence(filepointer, sequence, sizeofsequence) ;

        if (globals.veryverbose)
        {

            if (fileoffset > 0 )
            {

                EXPLAIN("%s%llu", "Found sequence at offset: ", fileoffset-sizeofsequence+1);

            }
            else if (fileoffset == 0)
            {

                EXPLAIN("%s%llu", "Missed sequence at offset: ",offset_save);

            }
            else

            {

                EXPLAIN("%s%llu", "Error: could not find sequence at offset: ",offset_save);
            }
        }
        if (fileoffset <= 0)
        {
            fileoffset=offset_save;
            fseek(filepointer, fileoffset, SEEK_SET);

        }
    }

}





/* pack_scr was taken from mplex (part of the mjpegtools) */
#define MARKER_MPEG2_SCR 1
ALWAYS_INLINE_GCC   void pack_scr(uint8_t scr_bytes[6],uint64_t SCR_base, uint16_t SCR_ext)
{

    uint8_t temp;
    uint64_t msb, lsb;

    msb = (SCR_base>> 32) & 1;
    lsb = SCR_base & (0xFFFFFFFF);

    temp = (MARKER_MPEG2_SCR << 6) | (msb << 5) | ((lsb >> 27) & 0x18) | 0x4 | ((lsb >> 28) & 0x3);
    scr_bytes[0]=temp;
    temp = (lsb & 0x0ff00000) >> 20;
    scr_bytes[1]=temp;
    temp = ((lsb & 0x000f8000) >> 12) | 0x4 | ((lsb & 0x00006000) >> 13);
    scr_bytes[2]=temp;
    temp = (lsb & 0x00001fe0) >> 5;
    scr_bytes[3]=temp;
    temp = ((lsb & 0x0000001f) << 3) | 0x4 | ((SCR_ext & 0x00000180) >> 7);
    scr_bytes[4]=temp;
    temp = ((SCR_ext & 0x0000007F) << 1) | 1;
    scr_bytes[5]=temp;
}

ALWAYS_INLINE_GCC void pack_pts_dts(uint8_t PTS_DTS_data[10],uint32_t pts, uint32_t dts)
{
    uint8_t p3,p2,p1,p0,d3,d2,d1,d0;

    p3=(pts&0xff000000)>>24;
    p2=(pts&0x00ff0000)>>16;
    p1=(pts&0x0000ff00)>>8;
    p0=pts&0xff;
    d3=(dts&0xff000000)>>24;
    d2=(dts&0x00ff0000)>>16;
    d1=(dts&0x0000ff00)>>8;
    d0=dts&0xff;

    PTS_DTS_data[0]=0x31|((p3&0xc0)>>5);              // [32],31,30
    PTS_DTS_data[1]=((p3&0x3f)<<2)|((p2&0xc0)>>6);    // 29,28,27,26,25,24 ,23,22
    PTS_DTS_data[2]=((p2&0x3f)<<2)|((p1&0x80)>>6)|1;  // 21,20,19,18,17,16, 15
    PTS_DTS_data[3]=((p1&0x7f)<<1)|((p0&0x80)>>7);    // 14,13,12,11,10,9,8, 7
    PTS_DTS_data[4]=((p0&0x7f)<<1)|1;                 // 6,5,4,3,2,1,0
    PTS_DTS_data[5]=0x11|((d3&0xc0)>>5);              // [32],31,30
    PTS_DTS_data[6]=((d3&0x3f)<<2)|((d2&0xc0)>>6);    // 29,28,27,26,25,24 ,23,22
    PTS_DTS_data[7]=((d2&0x3f)<<2)|((d1&0x80)>>6)|1;  // 21,20,19,18,17,16, 15
    PTS_DTS_data[8]=((d1&0x7f)<<1)|((d0&0x80)>>7);    // 14,13,12,11,10,9,8, 7
    PTS_DTS_data[9]=((d0&0x7f)<<1)|1;                 // 6,5,4,3,2,1,0
}

ALWAYS_INLINE_GCC  void pack_pts(uint8_t PTS_DTS_data[10],uint32_t pts)
{
    uint8_t p3,p2,p1,p0;

    p3=(pts&0xff000000)>>24;
    p2=(pts&0x00ff0000)>>16;
    p1=(pts&0x0000ff00)>>8;
    p0=pts&0xff;

    PTS_DTS_data[0]=0x21|((p3&0xc0)>>5);              // [32],31,30
    PTS_DTS_data[1]=((p3&0x3f)<<2)|((p2&0xc0)>>6);    // 29,28,27,26,25,24 ,23,22
    PTS_DTS_data[2]=((p2&0x3f)<<2)|((p1&0x80)>>6)|1;  // 21,20,19,18,17,16, 15
    PTS_DTS_data[3]=((p1&0x7f)<<1)|((p0&0x80)>>7);    // 14,13,12,11,10,9,8, 7
    PTS_DTS_data[4]=((p0&0x7f)<<1)|1;                 // 6,5,4,3,2,1,0
}




// parse AOB until finds PACK HEADER bytes and return pointer to pack header sequence last offset+1



inline void process_pack_header(FILE* fp,  uint64_t SCRint, const char* ioflag)
{

    uint8_t scr_bytes[6];
    uint8_t pack_stuffing_length_byte[1]= {0xf8};

    const char *WRITE="WRITE", *READ="READ", *IO;



    /*200806  patch Fabrice Nicol <fabnicol@users.sourceforge.net>
    * floor returns a double (type conversion error).
    * SCRint=floor(SCR);
    */

    pack_scr(scr_bytes,(SCRint/300),(SCRint%300));

    if (globals.veryverbose)
    {
        IO= (ioflag[0] == 'w')? WRITE : READ;

        EXPLAIN("%s%s%d\n", IO, " PACK HEADER into file, size is: ", 4+6+3+1);
    }

    uint8_t tab[4+6+3+1]= {0};


    memcpy(tab,pack_start_code,4);
    memcpy(tab+4,scr_bytes,6);
    memcpy(tab+4+6,program_mux_rate_bytes,3);
    memcpy(tab+4+6+3,pack_stuffing_length_byte,1);

    write_search_sequence(tab,sizeof(tab), fp,ioflag);



}

ALWAYS_INLINE_GCC  void process_system_header(FILE* fp,const char* ioflag)
{
    uint8_t header_length[2]= {0x00,0x0c};
    uint8_t rate_bound[3]= {0x80,0xc4,0xe1};
    uint8_t audio_bound[1]= {0x04};
    uint8_t video_bound[1]= {0xa0};
    uint8_t packet_rate_restriction_flag[1]= {0x7f};
    uint8_t stream_info1[3]= {0xb8, 0xc0, 0x40};
    uint8_t stream_info2[3]= {0xbd, 0xe0, 0x0a};

    EXPLAIN("%s%d\n","WRITE SYSTEM HEADER, size is: ", 4+2+3+1+1+1+3+3)


    uint8_t tab[4+2+3+1+1+1+3+3]= {0};
    memcpy(tab,system_header_start_code,4);
    memcpy(tab+4,header_length,2);
    memcpy(tab+4+2,rate_bound,3);
    memcpy(tab+4+2+3,audio_bound,1);
    memcpy(tab+4+2+3+1,video_bound,1);
    memcpy(tab+4+2+3+1+1,packet_rate_restriction_flag,1);
    memcpy(tab+4+2+3+1+1+1,stream_info1,3);
    memcpy(tab+4+2+3+1+1+3,stream_info2,3);

    write_search_sequence(tab,sizeof(tab), fp,ioflag);


}

ALWAYS_INLINE_GCC  void process_pes_padding(FILE* fp,uint16_t length,const char* ioflag)
{
    uint8_t packet_start_code_prefix[3]= {0x00,0x00,0x01};
    uint8_t stream_id[1]= {0xbe};
    uint8_t length_bytes[2];
    uint8_t ff_buf[2048];

    memset(ff_buf,0xff,sizeof(ff_buf));

    length-=6; // We have 6 bytes of PES header.

    length_bytes[0]=(length&0xff00)>>8;
    length_bytes[1]=(length&0xff);
    EXPLAIN("%s%d\n", "PES padding, size is: ", 3+1+2+length)


    uint8_t tab[3+1+2+length];
    memcpy(tab,packet_start_code_prefix,3);
    memcpy(tab+3,&stream_id,1);
    memcpy(tab+3+1,length_bytes,2);
    memcpy(tab+3+1+2,ff_buf,length);

    write_search_sequence(tab, sizeof(tab),fp,ioflag);

}

ALWAYS_INLINE_GCC  void process_audio_pes_header(FILE* fp, uint16_t PES_packet_len, uint8_t extension_flag, uint64_t PTS,const char* ioflag)
{
    uint8_t packet_start_code_prefix[3]= {0x00,0x00,0x01};
    uint8_t stream_id[1]= {0xbd}; // private_stream_1
    uint8_t PES_packet_len_bytes[2];
    uint8_t flags1[1]= {0x81}; // various flags - original_or_copy=1
    uint8_t flags2[1]= {0}; // various flags - contains pts_dts_flags and extension_flav
    uint8_t PES_header_data_length[1]= {0};
    uint8_t PES_extension_flags[1]= {0x1e}; // PSTD_buffer_flag=1
    uint8_t PTS_DTS_data[10];
    uint8_t PSTD_buffer_scalesize[2];
    uint16_t PSTD=10240/1024;

    /* Set PTS_DTS_flags in flags2 to 2 (top 2 bits) - PTS only*/
    flags2[0]=(2<<6);

    if (extension_flag)
    {
        PES_header_data_length[0]+=3;
    }

    PES_header_data_length[0]+=5;        // PTS only (and PSTD)

    flags2[0]|=extension_flag;  // PES_extension_flag=1

    PES_packet_len_bytes[0]=(PES_packet_len&0xff00)>>8;
    PES_packet_len_bytes[1]=PES_packet_len&0xff;

    EXPLAIN("%s%d%s%d%s%d\n","WRITE AUDIO PES HEADER, PES_packet_len: ", PES_packet_len, " extension_flag: ", extension_flag, " size is: ", 3+1+2+1+1+1+5+(extension_flag)*3)

    pack_pts(PTS_DTS_data,PTS);

    uint8_t tab[3+1+2+1+1+1+5+1+2]= {0};
    memcpy(tab,packet_start_code_prefix,3);
    memcpy(tab+3,stream_id,1);
    memcpy(tab+3+1,PES_packet_len_bytes,2);
    memcpy(tab+3+1+2,flags1,1);
    memcpy(tab+3+1+2+1,flags2,1);
    memcpy(tab+3+1+2+1+1,PES_header_data_length,1);
    memcpy(tab+3+1+2+1+1+1,PTS_DTS_data,5);

    if (extension_flag)
    {
        PSTD_buffer_scalesize[0]=0x60|((PSTD&0x1f00)>>8);
        PSTD_buffer_scalesize[1]=PSTD&0xff;

        memcpy(tab+3+1+2+1+1+5,PES_extension_flags,1);
        memcpy(tab+3+1+2+1+1+1+5+1,PSTD_buffer_scalesize,2);

    }

    write_search_sequence(tab, sizeof(tab)-(!extension_flag*3), fp,ioflag);

}

ALWAYS_INLINE_GCC  void process_lpcm_header(FILE* fp, int header_length,fileinfo_t* info, uint64_t pack_in_title, uint8_t counter,const char* ioflag)
{
    uint8_t sub_stream_id[1]= {0xa0};

    uint8_t LPCM_header_length[2];
    uint8_t first_access_unit_pointer[2];
    uint8_t unknown1[1]= {0x10};  // e.g. 0x10 for stereo, 0x00 for surround
    uint8_t sample_size[1]= {0x0f}; // 0x0f=16-bit, 0x1f=20-bit, 0x2f=24-bit
    uint8_t sample_rate[1]= {0x0f}; // 0x0f=48KHz, 0x1f=96KHz, 0x2f=192KHz,0x8f=44.1KHz, 0x9f=88.2KHz, 0xaf=176.4KHz
    uint8_t unknown2[1]= {0x00};
    uint8_t channel_assignment;  // The channel assignment - 0=C; 1=L,R; 17=L,R,C,lfe,Ls,Rs
    uint8_t unknown3[1]= {0x80};

    uint8_t zero[16];
    memset(zero, 0, 16);

    int frame_offset;
    uint64_t bytes_written;
    uint64_t frames_written;


    LPCM_header_length[0]=(header_length&0xff00)>>8;
    LPCM_header_length[1]=header_length&0xff;

    if (info->channels < 3)
    {
        unknown1[0]=0x10;
        switch (info->bitspersample)
        {
        case 16:
            sample_size[0]=0x0f;
            break;
            /*case 20:
                sample_size[0]=0x1f;
                break;*/
        case 24:
            sample_size[0]=0x2f;
            break;
        default:
            break;
        }
        switch (info->samplerate)
        {
        case 48000:
            sample_rate[0]=0x0f;
            break;
        case 96000:
            sample_rate[0]=0x1f;
            break;
        case 192000:
            sample_rate[0]=0x2f;
            break;
        case 44100:
            sample_rate[0]=0x8f;
            break;
        case 88200:
            sample_rate[0]=0x9f;
            break;
        case 176400:
            sample_rate[0]=0xaf;
            break;
        default:
            break;
        }
    }
    else
    {
        unknown1[0]=0x00;
        switch (info->bitspersample)
        {
        case 16:
            sample_size[0]=0x00;
            break;
            /*case 20:
                sample_size[0]=0x12;
                break;*/
        case 24:
            sample_size[0]=0x22;
            break;
        default:
            break;
        }
        switch (info->samplerate)
        {
        case 48000:
            sample_rate[0]=0x00;
            break;
        case 96000:
            sample_rate[0]=0x11;
            break;
        case 192000:
            sample_rate[0]=0x22;
            break;
        case 44100:
            sample_rate[0]=0x88;
            break;
        case 88200:
            sample_rate[0]=0x99;
            break;
        case 176400:
            sample_rate[0]=0xaa;
            break;
        default:
            break;
        }
    }
    channel_assignment = info->cga;

    if (pack_in_title==0)
    {
        frames_written=0;
        bytes_written=0;
    }
    else
    {
        bytes_written=(pack_in_title*info->lpcm_payload)-info->firstpackdecrement;
        // e.g., for 4ch, First pack is 1984, rest are 2000
        frames_written=bytes_written/info->bytesperframe;
        if (frames_written*info->bytesperframe < bytes_written)
        {
            frames_written++;
        }
    }

    frame_offset=(frames_written*info->bytesperframe)-bytes_written+header_length-1;
    first_access_unit_pointer[0]=(frame_offset&0xff00)>>8;
    first_access_unit_pointer[1]=frame_offset&0xff;

    int sizeoftab=1+1+2+2+1+1+1+1+1+1+header_length-8;

    EXPLAIN("%s%d%s%d\n","WRITE LPCM HEADER for ",info->bitspersample, " bits: size is ", sizeoftab)

    uint8_t tab[sizeoftab];
    memcpy(tab,sub_stream_id,1);
    memcpy(tab+1,&counter,1);
    memcpy(tab+1+1,LPCM_header_length,2);
    memcpy(tab+1+1+2,first_access_unit_pointer,2);
    memcpy(tab+1+1+2+2,unknown1,1);
    memcpy(tab+1+1+2+2+1,sample_size,1);
    memcpy(tab+1+1+2+2+1+1,sample_rate,1);
    memcpy(tab+1+1+2+2+1+1+1,unknown2,1);
    memcpy(tab+1+1+2+2+1+1+1+1,&channel_assignment,1);
    memcpy(tab+1+1+2+2+1+1+1+1+1,unknown3,1);
    memcpy(tab+1+1+2+2+1+1+1+1+1+1,zero,header_length-8);

    write_search_sequence(tab, sizeoftab,fp, ioflag);

}


ALWAYS_INLINE_GCC  uint64_t calc_PTS_start(fileinfo_t* info, uint64_t pack_in_title)
{
    double PTS;
    uint64_t PTSint;
    uint64_t bytes_written;

    if (pack_in_title==0)
    {
        PTS=585.0;
    }
    else
    {
        bytes_written=(pack_in_title*info->lpcm_payload)-info->firstpackdecrement;
        // e.g., for 4ch, First pack is 1984, rest are 2000
        PTS=((bytes_written*90000.0)/(info->bytespersecond*1.0))+585.0;
    }

    PTSint=floor(PTS);
    return(PTSint);
}



ALWAYS_INLINE_GCC  uint64_t calc_SCR(fileinfo_t* info, uint64_t pack_in_title)
{
    double SCR;
    uint64_t SCRint;


    if (info->bytespersecond == 0)
    {
        foutput("[WAR]  file %s has bytes per second=0\n", info->filename);
        return 0;
    }


    SCR=(pack_in_title <= 4 ? pack_in_title : 4)*(2048.0*8.0*90000.0*300.0)/10080000.0;

    if (pack_in_title>=4)
    {
        SCR+=((info->SCRquantity*300.0*90000.0)/info->bytespersecond)-42.85;
    }
    if (pack_in_title>=5)
    {
        SCR+=(pack_in_title-4.0)*((info->lpcm_payload*300.0*90000.0)/info->bytespersecond);
    }

    SCRint=floor(SCR);

    return(SCRint);
}


ALWAYS_INLINE_GCC uint64_t calc_PTS(fileinfo_t* info, uint64_t pack_in_title)
{
    double PTS;
    uint64_t PTSint;
    uint64_t bytes_written;
    uint64_t frames_written;


    if (pack_in_title==0)
    {
        PTS=585.0;
    }
    else
    {
        bytes_written=(pack_in_title*info->lpcm_payload)-info->firstpackdecrement;
        // e.g., for 4ch, First pack is 1984, rest are 2000
        frames_written=bytes_written/info->bytesperframe;
        if (frames_written*info->bytesperframe < bytes_written)
        {
            frames_written++;
        }
        // 1 48Khz-based frame is 1200Hz, 1 44.1KHz frame is 1102.5Hz
        PTS=((frames_written*info->bytesperframe*90000.0)/(info->bytespersecond*1.0))+585.0;
    }

    PTSint=floor(PTS);
    return(PTSint);
}

// Lee Feldkamp patch for version 09.09 build 12
// corrects differences with canon version starting with version 08.12 subsequent to multichannel authoring

int process_pes_packet(FILE* fp, fileinfo_t* info, uint8_t* audio_buf, int bytesinbuffer, uint64_t pack_in_title,int pack_in_file,const char* ioflag)
{
    uint64_t PTS;
    uint64_t SCR;
    int audio_bytes=0;
    static int cc;  // Continuity counter - reset to 0 when pack_in_file=0
    int lpcm_payload;


    lpcm_payload=(info->bitspersample == 16)? 2000 : 2004;


    PTS=calc_PTS(info,pack_in_title);
    SCR=calc_SCR(info,pack_in_title);

    if (pack_in_file==0)
    {
        cc=0;
    }

    if (pack_in_title==0)              // First packet in title
    {
        EXPLAIN("%s\n","WRITE PACK HEADER and PES HEADER: first-->")
        process_pack_header(fp,SCR, ioflag);
        process_system_header(fp, ioflag);
        process_audio_pes_header(fp,2010,1,PTS, ioflag);
        if (info->bitspersample==16)
        {
            audio_bytes=1984;
            process_lpcm_header(fp,0x0b,info,pack_in_title,cc, ioflag);
        }
        else if (info->bitspersample==24)
        {
            audio_bytes=1980;
            process_lpcm_header(fp,0x0f,info,pack_in_title,cc, ioflag);
        }

        EXPLAIN("%s%d%s\n","WRITE ", audio_bytes, " bytes of audio_buf in file (first)" )

        if (!globals.nooutput) fwrite (audio_buf,1,audio_bytes,fp);
    }
    else if (bytesinbuffer < lpcm_payload)   // Last packet in title
    {
        printf("[INF]  Writing last packet - pack=%lld, bytesinbuffer=%d\n",pack_in_title,bytesinbuffer);
        audio_bytes=bytesinbuffer;
        EXPLAIN("%s\n","write PACK HEADER and PES HEADER: last-->")
        process_pack_header(fp,SCR, ioflag);

        int dave_offset = DAVE_OFFSET;  // value of 6 forces agreement with Dave's code; LF recommends value of 0 which is more consistent with middle pack treatment
        process_audio_pes_header(fp,-dave_offset+info->midpack_audiopesheaderquantity-(lpcm_payload-audio_bytes),0,PTS,ioflag);
        process_lpcm_header(fp,-dave_offset+info->midpack_lpcm_headerquantity,info,pack_in_title,cc,ioflag);

        EXPLAIN("%s%d%s\n","WRITE ", audio_bytes, " bytes of audio_buf in file (last)" )


        if (!globals.nooutput) fwrite (audio_buf,1,audio_bytes,fp);
        process_pes_padding(fp,dave_offset+2048-14-14-20-audio_bytes,ioflag);

        // write_audio_pes_header(fp,2022-(2000-audio_bytes),0,PTS);
        // write_lpcm_header(fp,0x0a,info,pack_in_title,cc);
        // if (!globals.nooutput) fwrite (audio_buf,1,audio_bytes,fp);
        // write_pes_padding(fp,2000-audio_bytes+6);
    }
    else                             // A middle packet in the title.
    {
        audio_bytes=lpcm_payload;
        EXPLAIN("%s\n","write PACK HEADER and PES HEADER: middle-->")
        process_pack_header(fp,SCR,ioflag);
        process_audio_pes_header(fp,2028,0,PTS,ioflag);
        if (info->bitspersample==16)
        {
            process_lpcm_header(fp,0x10,info,pack_in_title,cc,ioflag);
        }
        else
        {

            process_lpcm_header(fp,0x0c,info,pack_in_title,cc,ioflag);
        }

        EXPLAIN("%s%d%s\n","WRITE ", audio_bytes, " bytes of audio_buf in file (middle)" )

        if (!globals.nooutput) fwrite(audio_buf,1,audio_bytes,fp);
    }

    if (cc==0x1f)
    {
        cc=0;
    }
    else
    {
        cc++;
    }
    return(audio_bytes);
}

void write_aob_path(char* aobfile,char* audiotsdir,int titleset,int filenum)
{
    STRING_WRITE_CHAR_BUFSIZ(aobfile, "%s"SEPARATOR"ATS_%02d_%d.AOB",audiotsdir,titleset,filenum)
}


int process_ats(char* audiotsdir,int titleset,fileinfo_t* files, int ntracks,const char* ioflag, char* player)
{

    FILE* aobfilepointer=NULL;
    char aobfile[CHAR_BUFSIZ+13+1];
    int i=0, pack=0, pack_in_file=0, filenum=1;
    uint32_t bytesinbuf=0, n=0, lpcm_payload=0;
    uint8_t audio_buf_in[AUDIO_BUFFER_SIZE];
    uint8_t audio_buf_out[AUDIO_BUFFER_SIZE];
    uint64_t pack_in_title=0;

//     if (player)
//     {
//
//         while (filenum < )
//         if ((extract) && (!extract->extracttitleset[filenum-1]))
//         {
//             filenum++;
//             continue;
//         }
//
//       {
//                     write_aob_path(aobfile, audiotsdir, titleset, filenum);
//                     char* argsplayer[]={aobfile, "vlc://quit", NULL};
//                     if (strcmp(player, "vlc") != 0) argsplayer[1]=NULL;
//                     if (run(player, argsplayer, 0) == -1)  // will wait for player until ends
//                     {
//                         EXPLAIN_DEV("[ERR]  Could not play group:", i)
//                         EXIT_ON_RUNTIME_ERROR
//                     }
//                     else
//                     {
//                         EXPLAIN_DEV("[ERR]  Played group:", i)
//                         return;
//                     }
//                     filenum++;
//
//        }
//        return 1-filenum;
//
//     }

  //extraction file by file may be necessary even for playback on some players

    write_aob_path(aobfile, audiotsdir, titleset, filenum);

    if (!globals.nooutput) aobfilepointer=secure_open(aobfile,ioflag);
    /* Open the first file and initialise the input audio buffer */


    if (audio_open(&files[i], ioflag)!=0)
        EXIT_ON_RUNTIME_ERROR_VERBOSE("[ERR]  Could not open file in process_ats.")

        n=process_audio(&files[i],audio_buf_in,audio_buf_out,AUDIO_BUFFER_SIZE-bytesinbuf,ioflag);
    bytesinbuf=n;

    lpcm_payload = files[i].lpcm_payload;

    files[i].first_sector=0;
    files[i].first_PTS=calc_PTS_start(&files[i],pack_in_title);

    printf("[INF]  Processing %s\n",files[i].filename); // to be initialized if ioflag="r"

    while (bytesinbuf)
    {
        if (bytesinbuf >= lpcm_payload)
        {
            //pack_in_file is not used in write_pes_packet
            n=process_pes_packet(aobfilepointer,&files[i],audio_buf_out,bytesinbuf,pack_in_title,pack_in_file,ioflag);

            memmove(audio_buf_out,&audio_buf_out[n],bytesinbuf-n);// could be replaced with audio_buf << 32*n
            bytesinbuf -= n;

            pack++;
            pack_in_title++;
            pack_in_file++;
        }

        if ((pack > 0) && ((pack%(512*1024))==0))
        {
            fclose(aobfilepointer);
            filenum++;
            write_aob_path(aobfile, audiotsdir, titleset, filenum);

            if (!globals.nooutput) aobfilepointer=fopen(aobfile,ioflag);
        }

        if (bytesinbuf < lpcm_payload)
        {


            if (player)
            {
                if (globals.debugging) foutput("[INF]  Extracting audio to buffer before playing with %s\n", player);
            }


            n=process_audio(&files[i],&audio_buf_in[bytesinbuf],&audio_buf_out[bytesinbuf],AUDIO_BUFFER_SIZE-bytesinbuf,ioflag);
            bytesinbuf+=n;
            if (n==0)   /* We have reached the end of the input file */
            {
                if (globals.veryverbose)
                {
                    foutput("[INF]  Audio processing g of track %d completed.\n", i);
                    if (player)
                        foutput("[INF]  Now playing with %s\n", player);
                }

                if (player)
                {
                    char* argsplayer[]={files[i].filename, "vlc://quit", NULL};
                    if (run(player, argsplayer, 0) == -1)  // will wait for player until ends
                        EXPLAIN_DEV("[ERR]  Could not play file", i)
                        EXIT_ON_RUNTIME_ERROR

                }

                files[i].last_sector=pack;
                audio_close(&files[i],ioflag);
                i++;
                pack_in_file=-1;

                if (i<ntracks)
                {
                    /* If the current track is a different audio format, we must start a new title. */
                    if ((files[i].samplerate!=files[i-1].samplerate) || (files[i].channels!=files[i-1].channels) || (files[i].bitspersample!=files[i-1].bitspersample) || (files[i].newtitle))
                    {
                        n=process_pes_packet(aobfilepointer,&files[i-1],audio_buf_out,bytesinbuf,pack_in_title,pack_in_file,ioflag); // Empty audio buffer.
                        pack++;
                        pack_in_title=0;
                        pack_in_file=0;
                        bytesinbuf=0;


                        files[i].first_PTS=calc_PTS_start(&files[i],pack_in_title);
                    }
                    else
                    {
                        files[i].first_PTS=calc_PTS_start(&files[i],pack_in_title+1);
                    }

                    files[i].first_sector=files[i-1].last_sector+1;
                    if (audio_open(&files[i],ioflag)!=0)
                    {
                        foutput("[ERR]  Could not open %s\n",files[i].filename);
                        EXIT_ON_RUNTIME_ERROR
                    }

                    n=process_audio(&files[i],&audio_buf_in[bytesinbuf],&audio_buf_out[bytesinbuf],AUDIO_BUFFER_SIZE-bytesinbuf,ioflag);
                    bytesinbuf+=n;
                    foutput("[INF]  Processing %s\n",files[i].filename);
                }
                else
                {
                    /* We have reached the last packet of the last file */
                    if (bytesinbuf==0)
                    {
                        files[i-1].last_sector=pack-1;
                    }
                    else
                    {
                        n=process_pes_packet(aobfilepointer,&files[i-1],audio_buf_out,bytesinbuf,pack_in_title,pack_in_file,ioflag); // Empty audio buffer.
                        bytesinbuf=0;
                        pack++;
                        pack_in_title++;
                    }
                }
            }
        }
    }

    if (files[0].single_track) files[0].last_sector=files[ntracks-1].last_sector;

    return(1-filenum);
}