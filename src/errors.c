#include "errors.h"
#include "main.h"

void nonExistingToken() {
    displayError(E_UNIMPLEMENTED);
}

void tokenWrongPlace() {
	displayError(E_WRONG_PLACE);
}

static const char *errors[] = {
    "This token is not implemented (yet)",
	"This token cannot be used at this place"
};

void displayError(unsigned int index) {
    // display 'errors[index]';
}
