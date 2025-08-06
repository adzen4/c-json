# C-json
Adzen's attempt at a JSON library. Warning: this is still in development and completely unusable.

## Quickstart

```c
#include "json.h"


int main(void)
{
	struct json_parse_result result;

	result = json_parse_file("config.json");
	if (result.error) {
		printf("Parsing error: %s\n", result.error);
	}

	config = result.value;

	...

	json_free(config);
}
```

