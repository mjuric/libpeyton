#include <astro/asteroids.h>
#include <astro/util.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

using namespace peyton;

// converts provisional designation to
// packed format
char *asteroids::prov2packed(char *dest, const char *_prov, char *orbitType)
{
	char pk[8], buf[10];
	char prov[20]; strcpy(prov, _prov);
	int st;

	// fist check if this thing has a comet prefix
	if(prov[1] == '/') {
		if(orbitType != NULL) *orbitType = prov[0];

		strcpy(prov, _prov + 2);
		char *c = strchr(prov, '('); // remove the discoverer if present
		if(c != NULL) *c = 0;

		char *s = strdup(prov);
		util::trim(prov, s);
		free(s);
	}

	// asteroid designation
	pk[7] = 0;

	// check for general format
	if(prov[6] == '-') {
		// survey name
		pk[0] = prov[5];
		pk[1] = prov[7];
		pk[2] = 'S';
		pk[3] = prov[0];
		pk[4] = prov[1];
		pk[5] = prov[2];
		pk[6] = prov[3];
	} else {
		strncpy(buf, prov, 2); buf[2] = 0;
		pk[0] = atoi(buf) + 55;
		pk[1] = prov[2];
		pk[2] = prov[3];
		pk[3] = prov[5];

		char *c;
		if((c = strchr(prov, '-')) == NULL) {
			// normal asteroid
			if(!isdigit(prov[6])) {
				pk[6] = prov[6]; // two letter code, XF for example
				st = 7;
			} else {
				pk[6] = '0';
				st = 6; // single letter, A1
			}

			// get index
			int subs;
			if(strlen(prov) == 7 && st != 6) {
				// no index - use 00
				subs = 0;
			} else {
				subs = atoi(prov + st);
			}

			// check for 100 overflow
			if(subs < 100) {
				sprintf(buf, "%02d", subs);
				pk[4] = buf[0];
				pk[5] = buf[1];
			} else {
				pk[4] = 55 + subs / 10;
				// check for 360 overflow - switch to lowercase letters
				if(pk[4] > 'Z') { pk[4] += 'a' - ('Z'+1); }
				pk[5] = '0' + (subs - 10 * (subs / 10));
			}
		} else {
			// comet with fragment
			*c = 0;
			int subs;
			subs = atoi(prov + 6);

			// check for 100 overflow
			if(subs < 100) {
				sprintf(buf, "%02d", subs);
				pk[4] = buf[0];
				pk[5] = buf[1];
			} else {
				pk[4] = 55 + subs / 10;
				pk[5] = '0' + (subs - 10 * (subs / 10));
			}

			pk[6] = ::tolower(*(c + 1));
		}
	}

	strcpy(dest, pk);
	return dest;
}

