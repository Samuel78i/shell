/////// PRATIQUE POUR SWITCH

typedef struct { char* key; int val; } internal_command;
#define sizeKeys (sizeof(table)/sizeof(internal_command))


// switch pour les commandes
internal_command table[] = {
        { "pwd", PWD }, { "cd", CD}, { "?", EXCL }, { "exit", EXIT },
        { "jobs", JOBS}, { "bg", BG }, { "fg", FG }, { "kill", KILL }
};

int key_from_str(char* key){
    for(int i = 0; i < sizeKeys; i++){
        internal_command t = table[i];
        if(strcmp(t.key, key) == 0){
            return t.val;
        }
    }
    return EXT;
}

// Commandes sur des char** ou char*

int size_len(char** array){
    int size = 0;

    while(array[size] != NULL){
        size++;
    }
    return size;
}

void rtrim(char *str) {
    int len = strlen(str);

    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t')) {
        str[--len] = '\0';
    }
}

void removeLastTab(char ** array) {
    if (array == NULL || array[0] == NULL) {
        return; // Nothing to remove
    }

    // Find the end of the array
    int size = size_len(array);

    // Mark the last element as removed
    free(array[size]);
    array[size] = NULL;
}

int is_redirection(char* commands){
    if(strcmp(commands, "<") == 0 ||
       strcmp(commands, ">") == 0 ||
       strcmp(commands, "2>") == 0 ||
       strcmp(commands, "2>>") == 0 ||
       strcmp(commands, ">>") == 0 ||
       strcmp(commands, "2>|") == 0 ||
       strcmp(commands, ">|") == 0 ||
       strcmp(commands, "|") == 0){
        return 1;
    }

    return 0;
}

bool aPipeIsPresent(char* str){
    if (str == NULL) {
        return false;
    }

    int length = strlen(str);

    for (int i = 0; i < length; ++i) {
        if (str[i] == '|') {
            // Check if the '|' is alone (not preceded or followed by other non-space characters)
            if ((i == 0 || str[i - 1] == ' ') && (i == length - 1 || str[i + 1] == ' ')) {
                return true;
            }
        }
    }

    return false;
}


char** to_chars(char* line, char* token){
    char* bis = strdup(line);
    char** commands = (char**)malloc(32*sizeof(char*));
    for(int i = 0; i < 32; i++){ commands[i] = NULL; }
    int i = 0;

    char* sep = strtok(bis, token);
    while(sep != NULL){
        commands[i] = realloc(commands[i], (strlen(sep) + 1) * sizeof(char));

        strcpy(commands[i], sep);
        sep = strtok(NULL, token);
        i++;
    }
    return commands;
}

char** trim(char** commands) {
    char** result = (char**)malloc(32*sizeof(char*));
    if (result == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    int n = size_len(commands);
    int i;
    for (i = 0; i<n; ++i) {
        if (commands[i] == NULL || is_redirection(commands[i])) break; // if()
        result[i] = commands[i];
    }

    result[i] = NULL;

    return result;
}

char* substringToLastClosingParenthesis(const char* input) {
    const char* lastClosingParenthesis = strrchr(input, ')');

    if (lastClosingParenthesis != NULL) {
        // Calculate the length of the substring up to the last ')'
        size_t substringLength = lastClosingParenthesis - input + 1;

        // Create a new string containing the substring
        char* result = (char*)malloc(substringLength + 1);
        strncpy(result, input, substringLength);
        result[substringLength] = '\0';

        return result;
    }

    // Return NULL if no closing parenthesis is found
    return NULL;
}


char* substringFromLastClosingParenthesis(const char* input) {
    int length = strlen(input);
    const char* lastClosingParenthesis = strrchr(input, ')');

    if (lastClosingParenthesis != NULL) {
        // Calculate the length of the substring from the last ')'
        size_t substringLength = length - (lastClosingParenthesis - input) - 1;

        // Create a new string containing the substring
        char* result = (char*)malloc(substringLength + 1);
        strncpy(result, lastClosingParenthesis + 1, substringLength);
        result[substringLength] = '\0';

        return result;
    }

    // Return NULL if no closing parenthesis is found
    return NULL;
}

int getLastParentheseIndex(char* input) {
    int length = strlen(input);
    // Iterate from the end of the string to find the last '('
    for (int i = length - 1; i >= 0; i--) {
        if (input[i] == ')') {
            return i;
        }
    }
    return -1;
}

void removeLastClosingParenthesis(char* input) {
    int length = strlen(input);

    // Iterate from the end of the string to find the last ')'
    for (int i = length - 1; i >= 0; i--) {
        if (input[i] == ')') {
            // Remove the last ')' by shifting the characters
            memmove(&input[i], &input[i + 1], length - i);
            break;
        }
    }
}

char* removeFirstOccurrence(const char* input) {
    // Find the position of "<("
    const char* start = strstr(input, "<( ");

    if (start == NULL) {
        // If "<(" is not found, return a copy of the input
        return strdup(input);
    }

    // Find the position of ")"
    const char* end = strstr(start, ")");

    if (end == NULL) {
        // If ")" is not found, return a copy of the input
        return strdup(input);
    }

    // Calculate the lengths of the prefix and suffix
    size_t prefixLength = start - input;
    size_t suffixLength = strlen(end);

    // Create a new string without the first occurrence of "<(" and ")"
    char* newString = (char*)malloc(prefixLength + suffixLength + 1);
    strncpy(newString, input, prefixLength);
    newString[prefixLength] = '\0';
    strcat(newString, start + 3);  // Skip "<( " when copying
    removeLastClosingParenthesis(newString);

    return newString;
}


// Fonction récursive pour remplacer "a<(b)" par "b | a"
char * replaceSubstringRecursive(char *input) {
    // Recherche de la première occurrence de la sous-chaîne à remplacer
    char *found = strstr(input, "<( ");

    // Si la sous-chaîne est trouvée, remplacez-la et récursivez
    if (found != NULL) {
        // Calcul de la longueur des parties avant et après la sous-chaîne
        size_t prefixLength = found - input;
        size_t suffixLength = strlen(found + 3); // Longueur de "<( " est 3

        // Création d'une nouvelle chaîne avec le remplacement
        char *newString = (char *)malloc(prefixLength + suffixLength + 1);
        char * withoutSub = removeFirstOccurrence(found);
        strcpy(newString, withoutSub);
        newString[strlen(withoutSub)] = '\0';

        char * result = (char *) malloc(  strlen(newString) + 1);
        char * substring = substringFromLastClosingParenthesis(newString);
        if(substring == NULL){
            strcpy(result, newString);
        }else{
            strcpy(result, substring);
        }
        strcat(result, " | ");
        strncat(result, input, prefixLength);


        char * sub = substringToLastClosingParenthesis(newString);
        if(sub == NULL){
            return  result;
        }
        return strcat(replaceSubstringRecursive(substringToLastClosingParenthesis(newString)), result); // Déplacer après "| "
    }
    return input;
}

bool substitution(char * input){
    if(strstr(input, "<(") != NULL){
        return 1;
    }
    return 0;
}