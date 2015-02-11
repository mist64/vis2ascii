/*
 * vis2ascii
 *   by Michael Steil, www.pagetable.com, 2004-09-19/20, 2015-02-11
 */

//#define DEBUG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DATASIZE 65536

FILE *f1, *f2;

static unsigned char
ascii_from_petscii(unsigned char c)
{
	if (c < 0x20) {
		c += 0x60;
	}
	return c;
}

static int
detect_visass(unsigned char *data, int len)
{
	unsigned char *d = data + 2;
	while (*d != 0xff) {
		d++;
	}

	int count = 5;
	while (d < data + len) {
#if 1
		if (!count--)
			return 1; // enough matches? -> OK (it's OK if file is broken later)
#endif
#ifdef DEBUG
		fprintf(stderr, "d[0] = %02x d[1] = %02x\n", d[0], d[1]);
#endif
		if (!d[1])
			return 1; // end of chain -> everything OK
		if (d[1] > 45)
			return 0; // line length implausible -> not VisAss
		if (d[0] != 0xff && d[0] != 0)
			return 0; // line does not start with $FF -> not VisAss

		d += d[1]; // next line
	}

	return 1;
}

static void
decode_visass(unsigned char *data, int len, FILE *f2)
{
	static char decode[] = "a11cpxcpyldxldystxstyaaxasrarraxsdcpdopisckillarlaxrlarraslosretopadcandaslbitbcsbeqbccbmibnebplbvsbvcbrkclccldcliclvcmpdecdexdeyeorincinxinyjmpjsrldalsrnoporaphaphpplaplprolrorrtirtssbcsecsedseistataxtaytsxtxatxstya.la.ba.by.br.tx.md.de.ma.st.wa.on.wo.kc";

	// skip load address
	unsigned char *d = data + 2;

	// some files contain unknown data at the beginning;
	// $FF marks the beginning of the first line
	while (*d != 0xff) {
		d++;
	}

	while (d[1]) {
#ifdef DEBUG
		fprintf(f2, "%02x %02x %02x |||", d[0], d[1], d[2]);
#endif
		if (d[2] != 0x55) {
			if (d[2] < 0x48) {
				fprintf(f2, "\t");
			}
			for (int i = 0; i < 3; i++) {
				fprintf(f2, "%c", decode[d[2] * 3 + i]);
			}
			fprintf(f2, " ");
		}

		for (unsigned char *s = &d[3]; s != &d[d[1]]; s++) {
			char c = ascii_from_petscii(*s);
			fprintf(f2, "%c", c);
		}
		fprintf(f2, "\n");

		d += d[1];
	}
}

static void
decode_f8assblaster(unsigned char *data, int len, FILE *f2)
{
	static char decode_blaster[] = "adcandaslbccbcsbeqbitbmibnebplbrabrkbrlbvcbvsclccldcliclvcmpcopcpxcpydeadecdexdeyeorinaincinxinyjmpjsljsrldaldxldylsrmvnmvpnoporapeapeiperphaphbphdphkphpphxphyplaplbpldplpplxplyreprolrorrtirtlrtssbcsecsedseisepstastpstxstystztaxtaytcdtcstdctrbtsbtsctsxtxatxstxytyatyxwaixbaxce.la.ba.by.br.tx.md.de.ma.st.wa.on.wo.kc.rl.rs.al.as";

	// skip load address
	unsigned char *d = data + 2;

	while (d < data + len) {
		unsigned char *d2 = &d[1];
		if (!d2[0])
			break;
#ifdef DEBUG
		for (unsigned char *dx = d + 1; *dx != 0xff && dx < data + len; dx++) {
			fprintf(f2, "%02x ", *dx);
		}
		fprintf(f2, "\n");
#endif
		unsigned char mode = 0;

		int is_label = d2[0] < 0x80 && d2[0] != ';' && d2[0] != ' ';

		if (d2[0] >= 0x80) {
			fprintf(f2, "\t");
			for (int i = 0; i < 3; i++) {
				fprintf(f2, "%c", decode_blaster[(d2[0] - 0x80) * 3 + i]);
			}
			fprintf(f2, " ");
			if (d2[0] >= 0xdc) {
				mode = 0;
				d2++;
			} else {
				mode = d2[1];
				d2 += 2;
			}
		}

		switch (mode) {
			case 0:
				break; // impl
			case 1:
				break; // abs
			case 2:
				break; // ,x
			case 3:
			case 0x13: // XXX
				break; // ,y
			case 4:
				fprintf(f2, "#");
				break;
			case 8:
				fprintf(f2, "(");
				break; // (ind),y
			case 9:
				fprintf(f2, "(");
				break; // (ind,x)
			case 0xa:
				break; // label
			case 0xb:
				fprintf(f2, "(");
				break; // indirect
			case 0xd: // XXX
				break;
				break;
			default:
				fprintf(stderr, "unknown mode $%02x\n", mode);
				exit(1);
		}

		for (; *d2 != 0xff && d2 < data + len; d2++) {
			if (*d2 == '\"') {
				fprintf(f2, "\"");
				d2++;
				for (; *d2 != '\"'; d2++) {
					fprintf(f2, "%c", ascii_from_petscii(*d2));
				}
				fprintf(f2, "\"");
			} else if (*d2 < 0x80) {
				fprintf(f2, "%c", ascii_from_petscii(*d2));
			} else {
				switch (*d2) {
					case 0x81: {
						fprintf(f2, "%%");
						for (int i = 7; i > 0; i--) {
							fprintf(f2, "%c", (1 << i) & d2[1] ? '1' : '0');
						}
						d2++;
						break;
					}
					case 0x85:
						fprintf(f2, "%d", d2[1]);
						d2++;
						break;
					case 0x86:
						fprintf(f2, "%d", d2[1] | (d2[2] << 8));
						d2 += 2;
						break;
					case 0x89:
						fprintf(f2, "$%02x", d2[1]);
						d2++;
						break;
					case 0x8a:
						fprintf(f2, "$%04x", d2[1] | (d2[2] << 8));
						d2 += 2;
						break;
					case 0x8b:
						fprintf(f2, "$%02x:$%04x", d2[3], d2[1] | (d2[2] << 8));
						d2 += 3;
						break;
					case 0x95:
						fprintf(f2, "%d", d2[1] + 1);
						d2++;
						break;
					case 0x96:
						fprintf(f2, "%d", (d2[1] + 1) | (d2[2] << 8));
						d2 += 2;
						break;
					case 0x99:
						fprintf(f2, "$%02x", d2[1] + 1);
						d2++;
						break;
					case 0x9a:
						fprintf(f2, "$%04x", (d2[1] + 1) | (d2[2] << 8));
						d2 += 2;
						break;
					case 0xaa:
						fprintf(f2, "$%04x", d2[1] | ((d2[2] + 1) << 8));
						d2 += 2;
						break;
					case 0xba:
						fprintf(f2, "$%04x", (d2[1] + 1) | ((d2[2] + 1) << 8));
						d2 += 2;
						break;
					default:
						fprintf(stderr, "unknown code $%02x\n", d2[0]);
						exit(1);
				}
			}
		}
		switch (mode) {
			case 0:
				break; // impl
			case 1:
				break; // abs
			case 2:
				fprintf(f2, ",x");
				break; // ,x
			case 3:
			case 0x13: // XXX
				fprintf(f2, ",y");
				break; // ,y
			case 4:
				break;
			case 8:
				fprintf(f2, "),y"); // (ind),y
				break;
			case 9:
				fprintf(f2, ",x)");
				break; // (ind,x)
			case 0xa:
				break; // label
			case 0xb:
				fprintf(f2, ")");
				break; // indirect
			case 0xd: // XXX
				break;
			default:
				fprintf(f2, "unknown mode $%02x\n", mode);
				exit(1);
		}

		if (is_label) {
			fprintf(f2, ":");
		}

		fprintf(f2, "\n");
		d = d2;
	}
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

	unsigned char data[DATASIZE];
	f1 = fopen(argv[1], "r");
	int len = fread(data, 1, DATASIZE, f1);
	fclose(f1);

	int visass = detect_visass(data, len);

	fprintf(stderr, "Converting \"%s\" (%s format) to ", argv[1], visass ? "VisAss" : "F8 AssBlaster");
	if (dest[0] == '-') {
		f2 = stdout;
		fprintf(stderr, "stdout...");
	} else {
		f2 = fopen(dest, "w");
		fprintf(stderr, "\"%s\"...", dest);
	}

	if (visass) {
		decode_visass(data, len, f2);
	} else {
		decode_f8assblaster(data, len, f2);
	}

	fclose(f2);
	fprintf(stderr, "done.\n");
}

