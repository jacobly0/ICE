#include "errors.h"
#include "main.h"

static const char *errors[] = {
    "This token is not implemented (yet)",
    "This token cannot be used at this place",
	"This token doesn't have a condition",
	"You used 'Else' or End' outside a condition block",
};

void displayError(unsigned int index) {
    // display 'errors[index]';
}
