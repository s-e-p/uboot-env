#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>


#define CRCLEN	4

#define PRE	0
#define GET	1
#define SET	2
#define DEL	3

#define errmsg(...)	fprintf(stderr, __VA_ARGS__)

typedef unsigned char	byte;
typedef	unsigned int	uint32;

int	reset;

int	file;

char *	config;		/* config file name */
char *	envname;	/* device or file name */
char *	savefile;	/* file to get/set */
char *	offsetv;	/* offset to start */
char *	lengthv;	/* length of environment */

long	offset;
long	length;
int	isdev;

void *	data;

void	usage(char * argv0)
{
	const char *	pname;

	if ((pname = rindex(argv0, '/')) == NULL)
		pname = argv0;

	else
		pname += 1;

	fprintf(stderr, "Usage: %s [-cdfol] get -s filename\n", pname);
	fprintf(stderr, "       %s [-cdfol] get [var1 ... ]\n", pname);
	fprintf(stderr, "       %s [-cdfol] del [var1 ... ]\n", pname);
	fprintf(stderr, "       %s [-cdfol] del -[i|I]\n", pname);
	fprintf(stderr, "       %s [-cdfol] set -s filename\n", pname);
	fprintf(stderr, "       %s [-cdfol] set var val\n", pname);
	fprintf(stderr, "       %s [-cdfol] set < uEnv.txt\n", pname);
	fprintf(stderr, "\n");
	fprintf(stderr, "      -c config        Alternate config file\n");
	fprintf(stderr, "      -d devicename    Device with environment\n");
	fprintf(stderr, "      -f filename      File with environment\n");
	fprintf(stderr, "      -s filename      Save to or set from file\n");
	fprintf(stderr, "      -o offset        Offset into device\n");
	fprintf(stderr, "      -l length        Length in device\n");
	fprintf(stderr, "      -i               Initialize environment\n");
	fprintf(stderr, "      -I               Forced initialize\n");
}

static uint32	crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32	crc32(uint32 crc, const byte *buf, size_t size)
{
	while (size--)
		crc = crc32_tab[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);

	return crc;
}

void	setcrc(byte * data, size_t len)
{
	*((uint32 *) data) = crc32(~0U, data + CRCLEN, len - CRCLEN) ^ ~0U;
}

int	chkcrc(byte * data, size_t len)
{
	return ((*((uint32 *) data) ^ ~0U)  ^
		crc32(~0U, data + CRCLEN, len - CRCLEN)) != 0;
}

long	getnum(char * v)
{
	char *	s;
	long	value;

	value = strtol(v, &s, 0);
	if ((v == s) || *s)
		value = -1;

	return value;
}

char *	next_line(FILE * config)
{
	int	len;
	int	buf;
	char *	line;

	len = 0;
	buf = 0;
	line = NULL;

	for (;;)
	{
		if ((len + 1) < buf)
		{
			if (fgets(line + len, buf - len, config) == NULL)
				break;

			len += strlen(line + len);

			if (line[len-1] == '\n')
			{
				line[len-1] = '\0';
				break;
			}
		}
		else if ((line = realloc(line, buf += 100)) == NULL)
			break;
	}

	if (len == 0)
	{
		free(line);
		line = NULL;
	}

	return line;
}

#define CONF_IGNORE	-1
#define CONF_DEFAULT	0
#define CONF_DEVICE	1
#define CONF_NAME	2
#define CONF_OFFSET	3
#define CONF_LENGTH	4
#define CONF_ERROR	5

const char *	config_name[CONF_ERROR] =
{
	"default",
	"device",
	"name",
	"offset",
	"length"
};

int	config_line(char * line, int matched, char ** val)
{
	char *	lnam;
	char *	lval;
	int	select;

	while ((*line == ' ') || (*line == '\t'))
		line += 1;

	if ((*line == '#') || (*line == '\0'))
		return CONF_IGNORE;

	lnam = line;

	while (*line && (*line != ' ') && (*line != '\t')  && (*line != '='))
		line += 1;

	if (*line == '\0')
		return CONF_ERROR;

	*line = '\0';
	line += 1;

	while (*line && ((*line == ' ') || (*line == '\t')))
		line += 1;

	if (*line == '\0')
		return CONF_ERROR;

	lval = line;

	for (select = 0; select < CONF_ERROR; select++)
		if (strcmp(lnam, config_name[select]) == 0)
			break;

	if ((select != CONF_NAME) && !matched)
			return CONF_IGNORE;

	*val = lval;

	lval[strcspn(lval, "# \t")] = '\0';

	return select;
}

void	read_config(char * config, int isdev)
{
	char *	line;
	char *	device;
	FILE *	f;
	int	matched;

	if (isdev && (envname != NULL))
		device = strdup(envname);

	else
		device = NULL;

	matched = 1;

	if ((f = fopen(config, "ro")) == NULL)
	{
		perror(config);
		return;
	}

	while ((line = next_line(f)))
	{
		char *	val;

		switch (config_line(line, matched, &val))
		{
		case CONF_IGNORE:
			break;

		case CONF_DEFAULT:
			if (isdev)
			{
				if (device == NULL)
					device = strdup(val);

				matched = 0;
			}
			break;

		case CONF_DEVICE:
			free(device);
			device = strdup(val);
			break;

		case CONF_NAME:
			if (device == NULL)
				matched = -1;

			else
				matched = (strcmp(val, device) == 0);

			break;

		case CONF_OFFSET:
			if (offsetv == NULL)
				offsetv = strdup(val);
			break;

		case CONF_LENGTH:
			if (lengthv == NULL)
				lengthv = strdup(val);
			break;

		default:
			fprintf(stderr, "Config file line invalid\n%s\n",
				line);
		}

		free(line);

		if (matched == -1)
			break;
	}

	if (isdev)
		envname = device;

	fclose(f);
}

int	process_args(int argc, char * argv[], int opt)
{
	int	arg;
	long	l;
	char **	setopt;


	for (arg = 1; arg < argc; arg++)
	{
		char *	c;

		c = argv[arg];

		if (*c++ != '-')
			break;

		while (*c)
		{

			switch (*c)
			{
			case 'i':
				reset = 1;
				c++;
				continue;

			case 'I':

				reset = -1;
				c++;
				continue;

			case 's':

				setopt = &savefile;
				break;

			case 'f':
				setopt = &envname;
				isdev = 0;
				break;

			case 'd':
				setopt = &envname;
				isdev = 1;
				break;

			case 'o':
				setopt = &offsetv;
				break;

			case 'l':
				setopt = &lengthv;
				break;

			case 'c':
				setopt = &config;
				break;

			default:
				errmsg("Invalid option\n");
				return -1;
			}

			if (*++c)
				*setopt = c;

			else if (++arg < argc)
				*setopt = argv[arg];

			else
			{
				errmsg("Missing argument\n");
				return -1;
			}

			break;
		}
	}

	if (opt == PRE)
		return arg;

	if ((reset != 0) && (opt != DEL))
	{
		errmsg("-i and -I can only be used with del\n");
		return -1;
	}

	if ((savefile != NULL) && (opt != GET) && (opt != SET))
	{
		errmsg("-s can only be used with get or set\n");
		return -1;
	}

	if (((savefile != NULL) || (reset != 0)) && (arg != argc))
	{
		errmsg("Invalid non-option arguments\n");
		return -1;
	}

	if ((lengthv == NULL) || (isdev &&
		((offsetv == NULL) || (envname == NULL))))
	{
		read_config(config, isdev);

		if ((lengthv == NULL) || (isdev &&
			(offsetv == NULL) || (envname == NULL)))
		{
			errmsg("Missing parameters or invalid device\n");
			return -1;
		}
	}

	if (offsetv == NULL)
		offset = 0;

	else if ((offset = getnum(offsetv)) < 0)
	{
		fprintf(stderr, "%s: invalid offset\n", offsetv);
		return -1;
	}

	if ((length = getnum(lengthv)) <= 0)
	{
		fprintf(stderr, "%s: invalid length\n", lengthv);
		return -1;
	}

	if ((file = open(envname, (opt == GET) ? O_RDONLY :
		O_RDWR | (isdev ? 0 : O_CREAT), 0666)) == -1)
	{
		perror(envname);
		return -1;
	}

	if ((data = malloc(length)) == NULL)
	{
		close(file);
		file = -1;

		fprintf(stderr, "Unable to allocate %lu bytes\n", length);
		return -1;
	}

	if ((l = pread(file, data, length, offset)) != length)
	{
		if (l == -1)
			perror("reading environment");

		else
			fprintf(stderr, "short read: %lu\n", l);

		l = 0;
	}
	else if (l == length)
	{
		if (chkcrc(data, length))
		{
			errmsg("CRC check failed\n");
			l = 0;
		}
	}

	if (((l == 0) && (reset == -1)) || ((l != 0) && (reset == 1)))
	{
		bzero(data, length);
		setcrc(data, length);
		errmsg("Cleared\n");
	}
	else if (l == 0)
		return -1;

	else if (reset == -1)
	{
		errmsg("-I not allowed on initialized environment\n");
		return -1;
	}

	if (opt == GET)
	{
		close(file);
		file = -1;
	}

	return arg;
}

int	next_entry(char * name, int * namel)
{
	int	length;
	int	l;

	l = 0;
	length = *namel;

	while ((l < length) && name[l] && (name[l] != '='))
		l += 1;

	if (name[l] != '=')
		return 0;

	*namel = l;
	name += l + 1;
	length -= l + 1;
	l = 0;

	while (l < length && name[l])
		l += 1;

	if (name[l])
		return 0;

	return l;
}

int	check(char * name, char * comp, int len)
{
	while (len > 0)
	{
		if (*comp < *name)
			return 1;

		if (*comp > *name)
			return -1;

		len -= 1;
		name += 1;
		comp += 1;
	}

	if (*comp)
		return 1;

	return 0;
}

int	match(char * name, int namel, char * list[], int listl)
{
	int	l;
	int	c;

	c = -1;

	for (l = 0; l < listl; l++)
		if ((c = check(name, list[l], namel)) == 0)
			break;

	return c;
}

int	set_env(char * name, char * val)
{
	int	l;
	int	loc;	/* where to update/insert */
	int	locl;	/* current length */
	int	namel;
	int	vall;
	int	adjust;
	char *	var;

	namel = strlen(name);
	vall = strlen(val);

	if (vall > 0)
		adjust = namel + vall + 2;
	else
		adjust = 0;

	loc = -1;

	l = CRCLEN;

	while ((l < length) && *(var = data + l))
	{
		int	varl;
		int	entl;

		varl = length - l;
		entl = next_entry(var, &varl);

		if (loc < 0)
		{
			int	c;

			for (c = 0; c < namel; c++)
				if (var[c] != name[c])
					break;


			if ((c == varl) && (c == namel))
			{
				loc = l;
				locl = varl + entl + 2;
			}
			else if (var[c] > name[c])
			{
				loc = l;
				locl = 0;
			}
		}

		l += varl + entl + 2;
	}

	if (loc < 0)
	{
		loc = l;
		locl = 0;
	}

	adjust = locl - adjust;
	locl += loc;

	if ((l - adjust) > length)
	{
		errmsg("environment full\n");
		return 1;
	}

	if ((adjust != 0) && (locl < l))
		memmove(data + locl - adjust, data + locl, l - locl);

	if (adjust > 0)
		bzero(data + l - adjust, adjust);

	if (vall > 0)
	{
		if (locl == loc)
		{
			memcpy(data + loc, name, namel + 1);
			((char *) data)[loc + namel] = '=';
		}

		memcpy(data + loc + namel + 1, val, vall + 1);
	}

	return 0;
}

int	read_env(FILE * file)
{
	char *	line;

	while ((line = next_line(file)) != NULL)
	{
		char *	sep;

		if ((sep = index(line, '=')) == NULL)
		{
			fprintf(stderr, "invalid input line:\n%s\n", line);
			return 1;
		}

		*sep = '\0';

		if (set_env(line, sep + 1))
			return 1;
	}
	
	return 0;
}

void	update(void)
{
        if (isdev) {
          mtd_info_t mtd_info;
          if (!ioctl(file, MEMGETINFO, &mtd_info)) {
            erase_info_t ei;
            ei.length = mtd_info.erasesize;
            ei.start = (offset/ei.length) * ei.length;          

            while((ssize_t)(ei.start - offset) < length) {
              ioctl(file, MEMUNLOCK, &ei);
              if (ioctl(file, MEMERASE, &ei)) {
                perror("Erase flash");
                exit(3);
              }
              ei.start += ei.length;
            }
          }
        }
    	setcrc(data, length);
	pwrite(file, data, length, offset);

	if (isdev)
		fdatasync(file);

	close(file);
	file = -1;
}

int	uenv_get(int argc, char * argv[])
{
	int	args;
	int	l;
	char *	var;

	if ((args = process_args(argc, argv, GET)) == -1)
		return 1;

	argc -= args;
	argv += args;

	l = CRCLEN;

	if (savefile == NULL)
	{
		while ((l < length) && *(var = data + l))
		{
			int	varl;
			char *	val;
			int	vall;

			varl = length - l;
			vall = next_entry(var, &varl);

			if (vall == 0)
			{
				fprintf(stderr, "Malformed environment\n");
				return 1;
			}

			val = var + varl + 1;

			if ((argc == 0) || (match(var, varl, argv, argc) == 0))
				printf("%.*s=%.*s\n", varl, var, vall, val);

			l += varl + vall + 2;
		}
	}
	else
	{
		int	save;
		int	l;

		l = -1;

		if (((save = open(savefile, O_RDWR | O_CREAT | O_TRUNC, 0666))
			== -1) || (((l = pwrite(save, data, length, 0)))
			!= length))
		{
			perror(savefile);
			fprintf(stderr, "%s: length written: %d\n", savefile, l);
			return 1;
		}
	}

	return 0;
}

int	uenv_set(int argc, char * argv[])
{
	int	args;

	if ((args = process_args(argc, argv, SET)) == -1)
		return 1;

	argc -= args;
	argv += args;

	if ((argc != 0) && (argc != 2))
	{
		errmsg("invalid\n");
		return 1;
	}

	if (savefile == NULL)
	{
		if (argc == 0)
		{
			if (read_env(stdin))
				return 1;
		}
		else if (argc == 2)
		{
			if (argv[0][strcspn(argv[0], "=")] != '\0')
			{
				errmsg("variable name invalid\n");
				return 1;
			}

			if (set_env(argv[0], argv[1]))
				return 1;
		}
	}
	else
	{
		int	save;
		int	l;

		l = -1;

		if (((save = open(savefile, O_RDONLY)) == -1) ||
			((l = pread(save, data, length, 0)) != length))
		{
			perror(savefile);
			fprintf(stderr, "%s: length read %d\n", savefile, l);
			return 1;
		}

		close(save);

		if (chkcrc(data, length))
		{
			errmsg("Invalid CRC\n");
			return 1;
		}
	}

	update();

	return 0;
}

int	uenv_del(int argc, char * argv[])
{
	int	args;
	int	l;
	char *	var;
	int	vall;

	int	dstart;
	int	dend;

	if ((args = process_args(argc, argv, DEL)) == -1)
		return 1;

	argc -= args;
	argv += args;

	dstart = 0;
	dend = 0;

	l = CRCLEN;

	while ((l < length) && *(var = data + l))
	{
		int	varl;
		int	entl;

		varl = length - l;
		entl = next_entry(var, &varl);

		if (entl == 0)
		{
			fprintf(stderr, "Malformed environment\n");
			return 1;
		}

		entl = l + varl + entl + 2;

		if (match(var, varl, argv, argc) == 0)
		{
			if (dstart == dend)
				dstart = l;

			else if (dend < l)
			{
				memmove(data + dstart, data + dend, l - dend);
				dstart += l - dend;
			}

			dend = entl;
		}

		l = entl;
	}

	if (dstart != dend)
	{
		if (dend < l)
			memmove(data + dstart, data + dend, l - dend);

		l -= dend - dstart;

		bzero(data + l, length - l);
	}

	update();

	return 0;
}

int	main(int argc, char * argv[])
{
	int	args;

	isdev = 1;		/* 0 - file; 1 - device */
	reset = 0;		/* 0 - no reset; 1 - reset; -1 - forced */
	savefile = NULL;	/* file name to save to or restore from */
	config = "/etc/uboot-env.conf";	/* ignored for now */
	envname = NULL;
	lengthv = offsetv = NULL;

	if ((args = process_args(argc, argv, PRE)) < 0)
		return 1;

	if ((argc - args) < 1)
	{
		usage(argv[0]);
		return 1;
	}

	if (strcmp(argv[args], "get") == 0)
		return uenv_get(argc - args, argv + args);

	if (strcmp(argv[args], "set") == 0)
		return uenv_set(argc - args, argv + args);

	if (strcmp(argv[args], "del") == 0)
		return uenv_del(argc - args, argv + args);

	usage(argv[0]);

	return 1;
}
