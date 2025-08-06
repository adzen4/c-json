# C-json
Adzen's attempt at a JSON library.

## Quickstart
	#include "json.h"

	struct server_config {
		char *name;
		char *directory;
		/* ... */
	};


	int main(void)
	{
		struct json_parse_result result;
		struct json_value json;
		struct server_config config;

		result = json_parse_file("server_config.json");
		if (result.error != NULL) {
			printf("Parsing error occured: %s\n", result.error);
			return 1;
		}
	}
