��    0      �  C         (     )     /     K  !   d     �     �     �  &   �     �       �     �   �  �   �  	   I     S     _     k     t     �     �     �     �     �  "   �  �   �     �     �     �  �   �  �   Z	     �	  1   �	     
     %
     .
  �   N
  B   )  1   l     �  5   �     �     �       �    )   �            �       �  6   �  3   3  5   g  3   �  3   �  .     K   4     �     �  3  �  �  �  #  ]  	   �     �     �     �     �     �     �     �     �        Y     k  h     �     �     �    �    r     w  G   �     �     �  <   �  q    �   �  t         �   v   �   
   !     $!     @!  �  V!  Q   #)     u)     x)                       	   &                                             /   )      +          
      $          #         "                   !         0      .                   (   %                       *           ,   '            -    %B %Y %s \- manual page for %s %s %s: can't create %s (%s) %s: can't get `%s' info from %s%s %s: can't open `%s' (%s) %s: can't unlink %s (%s) %s: error writing to %s (%s) %s: no valid information found in `%s' AUTHOR AVAILABILITY Additional material may be included in the generated output with the
.B \-\-include
and
.B \-\-opt\-include
options.  The format is simple:

    [section]
    text

    /pattern/
    text
 Any
.B [NAME]
or
.B [SYNOPSIS]
sections appearing in the include file will replace what would have
automatically been produced (although you can still override the
former with
.B --name
if required).
 Blocks of verbatim *roff text are inserted into the output either at
the start of the given
.BI [ section ]
(case insensitive), or after a paragraph matching
.BI / pattern /\fR.
 COPYRIGHT DESCRIPTION ENVIRONMENT EXAMPLES Environment Examples FILES Files Games INCLUDE FILES Include file for help2man man page Lines before the first section or pattern which begin with `\-' are
processed as options.  Anything else is silently ignored and may be
used for comments, RCS keywords and the like.
 NAME OPTIONS Options Other sections are prepended to the automatically produced output for
the standard sections given above, or included at
.I other
(above) in the order they were encountered in the include file.
 Patterns use the Perl regular expression syntax and may be followed by
the
.IR i ,
.I s
or
.I m
modifiers (see
.BR perlre (1)).
 REPORTING BUGS Report +(?:[\w-]+ +)?bugs|Email +bug +reports +to SEE ALSO SYNOPSIS System Administration Utilities The full documentation for
.B %s
is maintained as a Texinfo manual.  If the
.B info
and
.B %s
programs are properly installed at your site, the command
.IP
.B info %s
.PP
should give you access to the complete manual.
 The latest version of this distribution is available on-line from: The section output order (for those included) is: This +is +free +software Try `--no-discard-stderr' if option outputs to stderr Usage User Commands Written +by `%s' generates a man page out of `--help' and `--version' output.

Usage: %s [OPTION]... EXECUTABLE

 -n, --name=STRING       description for the NAME paragraph
 -s, --section=SECTION   section number for manual page (1, 6, 8)
 -m, --manual=TEXT       name of manual (User Commands, ...)
 -S, --source=TEXT       source of program (FSF, Debian, ...)
 -L, --locale=STRING     select locale (default "C")
 -i, --include=FILE      include material from `FILE'
 -I, --opt-include=FILE  include material from `FILE' if it exists
 -o, --output=FILE       send output to `FILE'
 -p, --info-page=TEXT    name of Texinfo manual
 -N, --no-info           suppress pointer to Texinfo manual
 -l, --libtool           exclude the `lt-' from the program name
     --help              print this help, then exit
     --version           print version number, then exit

EXECUTABLE should accept `--help' and `--version' options and produce output on
stdout although alternatives may be specified using:

 -h, --help-option=STRING     help option string
 -v, --version-option=STRING  version option string
 --version-string=STRING      version string
 --no-discard-stderr          include stderr when parsing option output

Report bugs to <bug-help2man@gnu.org>.
 help2man \- generate a simple manual page or other Project-Id-Version: help2man 1.40.7
Report-Msgid-Bugs-To: Brendan O'Dea <bug-help2man@gnu.org>
POT-Creation-Date: 2013-06-06 08:26+1000
PO-Revision-Date: 2012-05-05 22:23+0100
Last-Translator: Savvas Radevic <vicedar@gmail.com>
Language-Team: Greek <team@lists.gnome.gr>
Language: Greek
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=2; plural=(n != 1);
X-Poedit-Language: Greek
X-Poedit-Country: GREECE
 %B %Y %s \- σελίδα εγχειριδίου για %s %s %s: αδυναμία δημιουργίας %s (%s) %s: αδυναμία λήψης `%s' info από %s%s %s: αδυναμία ανοίγματος `%s' (%s) %s: αδυναμία αποσύνδεσης %s (%s) %s: σφάλμα εγγραφής σε %s (%s) %s: δεν βρέθηκαν έγκυρες πληροφορίες σε `%s' AUTHOR AVAILABILITY Επιπλέον υλικό μπορεί να συμπεριληφθεί στο παραγόμενο αποτέλεσμα με
.B \-\-include
και
.B \-\-opt\-include
ως επιλογές. Η μορφή είναι απλή:

    [ενότητα]
    κείμενο

    /μοτίβο/
    κείμενο
 Οποιεσδήποτε
.B [NAME]
ή
.B [SYNOPSIS]
ενότητες που εμφανίζονται στο αρχείο για να συμπεριληφθούν θα αντικαταστήσουν ό,τι θα είχε
παραχθεί αυτόματα (παρόλο που μπορείτε ακόμα να παρακάμψετε την
προηγούμενη με
.B --name
αν απαιτείται).
 Μπλοκ αυτολεξί κειμένου *roff εισάγονται στο αποτέλεσμα είτε στην
αρχή της δοσμένης
.BI [ section ]
(προσοχή στα πεζά-κεφαλαία), ή μετά από μοτίβο παραγράφου
.BI / pattern /\fR.
 COPYRIGHT DESCRIPTION ENVIRONMENT EXAMPLES Περιβάλλον Παραδείγματα FILES Αρχεία Παιχνίδια INCLUDE FILES Να συμπεριληφθεί αρχείο για τη σελίδα man του help2man Οι γραμμές πριν την πρώτη ενότητα ή μοτίβο που αρχίζουν με `\-' 
αντιμετωπίζονται ως επιλογές. Οτιδήποτε άλλο αγνοείται σιωπηρά και μπορεί
να χρησιμοποιηθεί για σχόλια, λέξεις κλειδιά RCS και παρόμοια.
 NAME OPTIONS Επιλογές Άλλες ενότητες ενσωματώνονται στο αυτόματα παραγόμενο αποτέλεσμα για
τις τυπικές ενότητες που δίνονται ανωτέρω, ή συμπεριλαμβάνονται στο
.I other
(ανωτέρω) με τη σειρά που είχαν στο αρχείο για να συμπεριληφθούν.
 Τα μοτίβα χρησιμοποιούν το συντακτικό κανονικών εκφράσεων της Perl και μπορεί να ακολουθούνται από
τους
.IR i ,
.I s
or
.I m
τροποποιητές (δες
.BR perlre (1)).
 REPORTING BUGS Αναφορά +(?:[\w-]+ +)?σφαλμάτων|Email +bug +reports +to SEE ALSO SYNOPSIS Εργαλεία διαχείρισης συστήματος Η πλήρης τεκμηρίωση για
.B %s
υπάρχει ως εγχειρίδιο Texinfo. Αν το
.B info
και
.B %s
τα προγράμματα έχουν εγκατασταθεί σωστά στη σελίδα σας, η εντολή
.IP
.B info %s
.PP
θα πρέπει να σας δίνει πρόσβαση στο πλήρες εγχειρίδιο.
 Η τελευταία έκδοση αυτής της διανομή είναι διαθέσιμη διαδικτυακά από: Η σειρά εξαγωγής ενοτήτων (γι' αυτές που περιλαμβάνονται) είναι: Αυτό +is +free +software Δοκιμάστε `--no-discard-stderr' αν η επιλογή στέλνει το αποτέλεσμα στο stderr Χρήση Εντολές χρήστη Συγγραφή: +by `%s' δημιουργεί μια σελίδα man από την έξοδο των `--help' και `--version' output.

Χρήση: %s [OPTION]... EXECUTABLE

 -n, --name=STRING       περιγραφή για την παράγραφο NAME
 -s, --section=SECTION   αριθμός ενότητας για σελίδα εγχειριδίου (1, 6, 8)
 -m, --manual=TEXT       όνομα εγχειριδίου (Εντολές Χρήστη, ...)
 -S, --source=TEXT       προέλευση προγράμματος (FSF, Debian, ...)
 -L, --locale=STRING     επιλογή τοποθεσίας (προεπιλογή "C")
 -i, --include=FILE      να συμπεριληφθεί υλικό από το `FILE'
 -I, --opt-include=FILE  να συμπεριληφθεί υλικό από το `FILE' εάν υπάρχει
 -o, --output=FILE       αποστολή αποτελέσματος σε `FILE'
 -p, --info-page=TEXT    όνομα του εγχειριδίου Texinfo
 -N, --no-info           καταστολή δείκτη σε εγχειρίδιο Texinfo
     --help              εμφάνιση αυτής της βοήθειας, μετά έξοδος
     --version           εμφάνιση αριθμού έκδοσης, μετά έξοδος

Το EXECUTABLE θα πρέπει να δέχεται επιλογές `--help' και `--version' και να παράγει αποτέλεσμα στην
stdout παρόλο που μπορεί να ορισθούν εναλλακτικά με χρήση:

 -h, --help-option=STRING     αλφαριθμητικό επιλογής βοήθεια
 -v, --version-option=STRING  αλφαριθμητικό επιλογής έκδοσης
 --version-string=STRING      αλφαριθμητικό έκδοσης
 --no-discard-stderr          να συμπεριληφθεί το stderr κατά την ανάλυση του αποτελέσματος της επιλογής

Αναφέρετε σφάλματα σε <bug-help2man@gnu.org>.
 help2man \- δημιουργία απλής σελίδας εγχειριδίου ή άλλο 