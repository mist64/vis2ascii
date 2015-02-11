/*
 * vis2ascii
 *   by Michael Steil, 19.-20.09.2004
 *
 * converts a Commodore 64 VisAss assembly source file
 * into ASCII
 *
 * Usage:
 * 1) vis2ascii "test		.src"
 *    - creates test.asm
 * 2) vis2ascii "test       .src" test.txt
 *    - creates test.txt
 * 3) vis2ascii "text       .src" -
 *    - prints text to stdout
 *
 * Notes:
 * a) vis2ascii automatically strips the spaces between the
 *    name and the extension if no target name is given.
 * b) vis2ascii cannot extract files from .D64 images. Use
 *    a tool like c1541 to extract the files:
 *      echo extract | c1541 disk.d64
 *      for i in *.src; do vis2ascii "$i"; done
 * c) vis2ascii only converts the Vis-Ass file format into
 *    ASCII, it does not convert the assembly conventions
 *    into another format. That is, pseudo-opcodes and
 *    macros stay in Vis-Ass format, but converting this
 *    manually should be easy.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DATASIZE 65536

static char decode[] = "a11cpxcpyldxldystxstyaaxasrarraxsdcpdopisckillarlaxrlarraslosretopadcandaslbitbcsbeqbccbmibnebplbvsbvcbrkclccldcliclvcmpdecdexdeyeorincinxinyjmpjsrldalsrnoporaphaphpplaplprolrorrtirtssbcsecsedseistataxtaytsxtxatxstya.la.ba.by.br.tx.md.de.ma.st.wa.on.wo.kc";

FILE *f1, *f2;

static void
cbmprintf(unsigned char *ss)
{
	for (unsigned char *s = ss; *s; s++) {
		char c = *s;
		if (c < 0x20) {
			c += 0x60;
		}
		fprintf(f2, "%c", c);
	}
	fprintf(f2, "\n");
}

int
main(int argc, char **argv)
{
	char dest[256];
	switch (argc) {
		case 2: {
			char *s;
			char *t;
			strcpy(dest, argv[1]);
			s = strrchr(dest, (int)'.');
			if (!s) {
				printf("Source file name does not end in \".src\".\n");
				exit(1);
			}
			for (t = s - 1; t >= dest; t--) {
				if (*t != ' ') break;
			}
			t++;
			strcpy(t, ".asm");
			break;
		}
		case 3:
			strcpy(dest, argv[2]);
			break;
		default:
			printf("Usage: vis2ascii source.src source.txt\n");
			exit(1);
	}

	fprintf(stderr, "Converting \"%s\" to ", argv[1]);
	if (dest[0] == '-') {
		f2 = stdout;
		fprintf(stderr, "stdout...");
	} else {
		f2 = fopen(dest, "w");
		fprintf(stderr, "\"%s\"...", dest);
	}

	unsigned char data[DATASIZE];
	f1 = fopen(argv[1], "r");
	fread(data, 1, DATASIZE, f1);
	fclose(f1);

	unsigned char *d = data + 2;
	while (d[1]) {
//		fprintf(f2, "%02x %02x %02x |||", d[0], d[1], d[2]);

		if (d[2] != 0x55) {
			if (d[2] < 0x48) {
				fprintf(f2, "\t");
			}
			for (int i = 0; i < 3; i++) {
				fprintf(f2, "%c", decode[d[2] * 3 + i]);
			}
			fprintf(f2, " ");
		}

		d[d[1]] = 0;
		cbmprintf(&d[3]);

		d += d[1];
	}
	fclose(f2);
	fprintf(stderr, "done.\n");
}
