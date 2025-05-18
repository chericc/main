// refer to https://github.com/schiermike/h264-sei-parser/tree/master
// 

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "xlog.h"

typedef struct {
	int payload_type; // [out]
	int payload_size; // [in][out]
	uint8_t *payload; // [in][out]
} SeiInfo;

void parse_sei_unit(FILE* fp, SeiInfo *sei_info) {
	int payload_type = 0;
	int payload_size = 0;

	do {
		while (!feof(fp) && payload_type % 0xFF == 0)
			payload_type += fgetc(fp);
		
		while (!feof(fp) && payload_size % 0xFF == 0)
			payload_size += fgetc(fp);

		int i;
		uint8_t *payload = sei_info->payload;
		for (i = 0; 
				i < payload_size 
				&& i < sei_info->payload_size 
				&& !feof(fp); 
				i++) {
			payload[i] = fgetc(fp);
		}

		sei_info->payload_type = payload_type;
		sei_info->payload_size = i;

	} while (0);

}

int seek_sei(FILE *fp, uint8_t *output, int *size) {
	int got_result_flag = 0;
	int checklen = 0;
	const int sep_len = 4;

	SeiInfo sei_info = {};

	while (!feof(fp)) {
		if ((checklen < sep_len - 1) && fgetc(fp) == 0x00) {
			checklen++;
		} else if ((checklen == sep_len - 1) && fgetc(fp) == 0x01) {
			checklen++;
		} else if (checklen == sep_len && fgetc(fp) == 0x06) {
			sei_info.payload = output;
			sei_info.payload_size = *size;
			parse_sei_unit(fp, &sei_info);
			got_result_flag = 1;
			*size = sei_info.payload_size;
			checklen = 0;
		} else {
			checklen = 0;
		}

		if (got_result_flag) {
			break;
		}
	}
	return (got_result_flag == 1) ? 0 : -1;
}

int parse_file(const char *filename)
{
	FILE *fp = NULL;
	void *buffer = NULL;
	FILE *fp_buf = NULL;
	do {
		fp = fopen(filename, "r");
		if (!fp) {
			xlog_err("open failed");
			break;
		}
		fseek(fp, 0, SEEK_END);
		long len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		buffer = malloc(len);
		if (!buffer) {
			xlog_err("malloc failed");
			break;
		}

		size_t ret_read = fread(buffer, 1, len, fp);
		if ((int)ret_read != len) {
			xlog_err("read failed\n");
			break;
		}

		fp_buf = fmemopen(buffer, len, "r");
		if (!fp_buf) {
			xlog_err("open failed");
			break;
		}

		uint8_t output[64];
		int output_len = sizeof(output) - 1;
		output[sizeof(output) - 1] = 0;

		int ret = seek_sei(fp_buf, output, &output_len);
		if (ret < 0) {
			xlog_err("parse sei failed");
			break;
		}

		xlog_dbg("size: %d", output_len);
		xlog_dbg("output: <%s>", output);

	} while (0);

	if (fp) {
		fclose(fp);
		fp = NULL;
	}
	if (fp_buf) {
		fclose(fp_buf);
		fp_buf = NULL;
	}
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}

	return 0;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		xlog_err("Usage: %s <h264_file>\n", argv[0]);
	} else {
		parse_file(argv[1]);
	}

	return 0;
}
