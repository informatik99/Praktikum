#ifndef PRAKTIKUM_CONSTANTS_H
#define PRAKTIKUM_CONSTANTS_H
/*
 * Diese Datei ist alleine dazu da,
 * global Konstanten zu setzen,
 * die für den server wichtig sein können
 * mehr auch nicht...
 */

// unser server kommuniziert
// über genau diesen port
#define SERVER_PORT 5678

// maximale text länge,
// die ein schlüssel haben kann
#define MAX_KEY_LENGTH 50

// maximale text länge,
// die der wert eines schlüssel-wert-paares haben kann
#define MAX_VALUE_LENGTH 100

// maximale text länge die eine nachricht haben kann
// die vom client gelesen wird, oder die der server ausgibt
#define MAX_MESSAGE_LENGTH 1024

// so viele schlüssel-wert-paare können wir speichern
#define MAX_STORE_KEY_VALUE_ENTRIES 50

// so viele subscriptions
// können wir insgesamt haben
#define MAX_SUBSCRIPTIONS 10

// nach so und so vielen verbindungsanfragen
// schließt der server
#define MAX_NUM_OF_CLIENT_CONNECTIONS 1

/**
 * FehlerCodes sollten überall einheitlich
 * immer kleiner als 0 sein.
 * Wenn alles in Ordnung war wird 0
 * zurückgegeben
 */
typedef enum StatusCode
{
    STATUS_FAILED = -5,
    STATUS_TEST_FAIL = -4,
    STATUS_KEY_NOT_EXISTENT = -3,
    STATUS_FAIL_EMPTY = -2,
    STATUS_FAIL_NOT_ENOUGH_SPACE = -1,
    STATUS_OK = 1
} StatusCode;

/////////// ANSI COLOR CODES ///////////////////
///// falls man die texte farbig gestalten möchte.....
// copied from: https://gist.github.com/RabaDabaDoba/145049536f815903c79944599c6f952a
/* Copyright notice:
 * This is free and unencumbered software released into the public domain.
 * For more information, please refer to <https://unlicense.org>
 */

//Regular text
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

//Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

//Regular underline text
#define UBLK "\e[4;30m"
#define URED "\e[4;31m"
#define UGRN "\e[4;32m"
#define UYEL "\e[4;33m"
#define UBLU "\e[4;34m"
#define UMAG "\e[4;35m"
#define UCYN "\e[4;36m"
#define UWHT "\e[4;37m"

//Regular background
#define BLKB "\e[40m"
#define REDB "\e[41m"
#define GRNB "\e[42m"
#define YELB "\e[43m"
#define BLUB "\e[44m"
#define MAGB "\e[45m"
#define CYNB "\e[46m"
#define WHTB "\e[47m"

//High intensty background
#define BLKHB "\e[0;100m"
#define REDHB "\e[0;101m"
#define GRNHB "\e[0;102m"
#define YELHB "\e[0;103m"
#define BLUHB "\e[0;104m"
#define MAGHB "\e[0;105m"
#define CYNHB "\e[0;106m"
#define WHTHB "\e[0;107m"

//High intensty text
#define HBLK "\e[0;90m"
#define HRED "\e[0;91m"
#define HGRN "\e[0;92m"
#define HYEL "\e[0;93m"
#define HBLU "\e[0;94m"
#define HMAG "\e[0;95m"
#define HCYN "\e[0;96m"
#define HWHT "\e[0;97m"

//Bold high intensity text
#define BHBLK "\e[1;90m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHBLU "\e[1;94m"
#define BHMAG "\e[1;95m"
#define BHCYN "\e[1;96m"
#define BHWHT "\e[1;97m"

//Reset
#define reset "\e[0m"
#define CRESET "\e[0m"
#define COLOR_RESET "\e[0m"

#endif //PRAKTIKUM_CONSTANTS_H
