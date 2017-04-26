#include "errors.h"
#include "main.h"

void nonExistingToken() {
    displayError(E_UNIMPLEMENTED);
}

void syntaxError() {
	displayError(E_SYNTAX);
}

static const char *errors[] = {
    "This token is not implemented (yet)",
	"Syntax error"
};

void displayError(unsigned int index) {
    // display 'errors[index]';
}
