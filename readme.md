# C-json
Adzen's attempt at a JSON library. Warning: this is still in development and completely unusable.

## Quickstart

```c
#include <stdio.h>
#include "json.h"
#include "my-server-library.h"

struct server_config {
	char *name;
	char *directory;
	int port;
	/* ... */
};

#define mapping(field, type) { #field, offsetof(struct server_config, field), type }
#define array_length(array) (sizeof(array) / sizeof(array[0]))

struct json_mapping mappings[] = {
	mapping(name, json_type_string),
	mapping(directory, json_type_string),
	mapping(port, json_type_number),
}

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

	config = new_default_server_config();

	err = json_map(mappings, array_length(mappings), &config);
	if (err != NULL) {
		printf("Failed to load configuration: %s\n, err");
	}

	start_server(&config);

	json_free(json);
}
```
