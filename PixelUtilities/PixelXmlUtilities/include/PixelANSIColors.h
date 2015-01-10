/*! \file PixelANSIColors.h
 *  \brief Header file containing macros to highlight output lines in color.<BR>
 *  Check also <A HREF="http://www.bluesock.org/~willg/dev/ansi.html">http://www.bluesock.org/~willg/dev/ansi.html</A>
 *  \image html separator.gif
 *  \image latex separator.pdf
 */
 
#ifndef _PIXELANSICOLORS_H_
#define _PIXELANSICOLORS_H_
#define ACBlack 	"\e[0;30m"
#define ACBlue		"\e[0;34m"
#define ACGreen 	"\e[0;32m"
#define ACCyan		"\e[0;36m"
#define ACRed		"\e[0;31m"
#define ACPurple	"\e[0;35m"
#define ACBrown 	"\e[0;33m"
#define ACGray		"\e[0;37m"
#define ACDarkGray	"\e[1;30m"
#define ACLightBlue	"\e[1;34m"
#define ACLightGreen	"\e[1;32m"
#define ACLightCyan	"\e[1;36m"
#define ACLightRed	"\e[1;31m"
#define ACLightPurple	"\e[1;35m"
#define ACYellow	"\e[1;33m"
#define ACWhite 	"\e[1;37m"

#define ACPlain 	"\e[0m"
#define ACBold		"\e[1m"
#define ACUnderline	"\e[4m"
#define ACBlink 	"\e[5m"
#define ACReverse	"\e[7m"

#define ACClear 	"\e[2J"
#define ACClearL	"\e[2K"

#define ACSave		"\e[s"
#define ACRecall	"\e[u"

#endif
