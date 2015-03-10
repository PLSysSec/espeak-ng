/*
 * Copyright (C) 2012-2015 Reece H. Dunn
 *
 * This file is part of ucd-tools.
 *
 * ucd-tools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ucd-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ucd-tools.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ucd/ucd.h"

#include <string.h>
#include <stdio.h>

bool fget_utf8c(FILE *in, ucd::codepoint_t &c)
{
	int ch = EOF;
	if ((ch = fgetc(in)) == EOF) return false;
	if (uint8_t(ch) < 0x80)
		c = uint8_t(ch);
	else switch (uint8_t(ch) & 0xF0)
	{
	default:
		c = uint8_t(ch) & 0x1F;
		if ((ch = fgetc(in)) == EOF) return false;
		c = (c << 6) + (uint8_t(ch) & 0x3F);
		break;
	case 0xE0:
		c = uint8_t(ch) & 0x0F;
		if ((ch = fgetc(in)) == EOF) return false;
		c = (c << 6) + (uint8_t(ch) & 0x3F);
		if ((ch = fgetc(in)) == EOF) return false;
		c = (c << 6) + (uint8_t(ch) & 0x3F);
		break;
	case 0xF0:
		c = uint8_t(ch) & 0x07;
		if ((ch = fgetc(in)) == EOF) return false;
		c = (c << 6) + (uint8_t(ch) & 0x3F);
		if ((ch = fgetc(in)) == EOF) return false;
		c = (c << 6) + (uint8_t(ch) & 0x3F);
		if ((ch = fgetc(in)) == EOF) return false;
		c = (c << 6) + (uint8_t(ch) & 0x3F);
		break;
	}
	return true;
}

void uprintf_codepoint(FILE *out, ucd::codepoint_t c, char mode)
{
	switch (mode)
	{
	case 'h': // hexadecimal (lower)
		fprintf(out, "%06x", c);
		break;
	case 'H': // hexadecimal (upper)
		fprintf(out, "%06X", c);
		break;
	}
}

void uprintf(FILE *out, ucd::codepoint_t c, const char *format)
{
	while (*format) switch (*format)
	{
	case '%':
		switch (*++format)
		{
		case 'c': // category
			fputs(ucd::get_category_string(ucd::lookup_category(c)), out);
			break;
		case 'C': // category group
			fputs(ucd::get_category_group_string(ucd::lookup_category_group(c)), out);
			break;
		case 'p': // codepoint
			uprintf_codepoint(out, c, *++format);
			break;
		case 'L': // lowercase
			uprintf_codepoint(out, ucd::tolower(c), *++format);
			break;
		case 's': // script
			fputs(ucd::get_script_string(ucd::lookup_script(c)), out);
			break;
		case 'T': // titlecase
			uprintf_codepoint(out, ucd::totitle(c), *++format);
			break;
		case 'U': // uppercase
			uprintf_codepoint(out, ucd::toupper(c), *++format);
			break;
		case 'W': // whitespace
			if (ucd::isspace(c))
				fputs("White_Space", out);
			break;
		}
		++format;
		break;
	default:
		fputc(*format, out);
		++format;
		break;
	}
}

void print_file(FILE *in)
{
	ucd::codepoint_t c = 0;
	while (fget_utf8c(in, c))
		uprintf(stdout, c, "%pH %s %C %c %UH %LH %TH %W\n");
}

int main(int argc, char **argv)
{
	if (argc == 2)
	{
		if (!strcmp(argv[1], "--stdin") || !strcmp(argv[1], "-"))
			print_file(stdin);
		else
		{
			FILE *in = fopen(argv[1], "r");
			if (in)
			{
				print_file(in);
				fclose(in);
			}
			else
				fprintf(stdout, "cannot open `%s`\n", argv[1]);
		}
	}
	else
	{
		for (ucd::codepoint_t c = 0; c <= 0x10FFFF; ++c)
			uprintf(stdout, c, "%pH %s %C %c %UH %LH %TH %W\n");
	}
	return 0;
}
