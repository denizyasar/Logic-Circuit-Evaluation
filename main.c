#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const static char * CIRCUIT_FILE = "circuit.txt";
const static char * INPUT_FILE = "input.txt";
const static char * GATE = "GATE";

enum {GATENAME_SIZE = 20, MAX_STRING = 1024};   /* define maximum string sizes */
enum {false, true};                             /* define bool values */
typedef enum  {INPUT, OUTPUT, AND,              /* define gate type as enum */
               OR, NOT, FLIPFLOP
              } GATETYPE;

struct gate
{
    GATETYPE type;                          /* type of the gate */
    char * name[GATENAME_SIZE];             /* name of the gate */
    struct gate ** inputs;                  /* points to a dynamically created array of pointers which store the addresses of input gate structs */
    int nInputs;                            /* number of inputs for iterations */
    int nOutput;                            /* stores the current output. this may be useful if the gate is connected to more than one gate.*/
    int former_out;                         /* only for flipflop */
    struct gate * nextGate;                 /* keep next gate for linked list structure */
    struct gate * output;                   /* keep output gate */
};

struct gate * head=NULL;                    /* linked list head gate */
struct gate * current=NULL;                 /* linked list current gate */

/*
* Function:  str_to_enum
* -----------------------
* converts read string value from file to gatetype enum
*
*  str: string value that will be converted
*
*  returns: GATETYPE enum value
*/

GATETYPE str_to_enum (char * str)
{
    if (strcmp (str,"INPUT") == 0)
    {
        return INPUT;
    }
    else if (strcmp (str,"OUTPUT") == 0)
    {
        return OUTPUT;
    }
    else if (strcmp (str,"AND") == 0)
    {
        return AND;
    }
    else if (strcmp (str,"OR") == 0)
    {
        return OR;
    }
    else if (strcmp (str,"NOT") == 0)
    {
        return NOT;
    }
    else if (strcmp (str,"FLIPFLOP") == 0)
    {
        return FLIPFLOP;
    }

    return 0;
}

/*
* Function:  find_gate_by_name
* --------------------
* finds the gate in linked list
*
*  name: name of the gate that will be searched for
*
*  returns: the gate with the name
*/

struct gate * find_gate_by_name(char * name)
{
    struct gate * result=head;
    /* loop through linked list until
    the matching gate with the name found */
    while((result != NULL) && (strcmp ((char*)result->name,name)))
    {
        result=result->nextGate;
    }
    return result;
}

/*
* Function:  assign_input_values
* -------------------------------
* assigns the input values read from the input text file
* to the INPUT gates by order
*
*  line: read line from the input text file
*
*  returns: 0
*/

int assign_input_values(char line[])
{
    struct gate * tmp=head;
    /*number of input values = length of input array
    * minus the last /0 char*/
    int input_count=strlen(line)-1;
    int i = 0;

    /*  */
    while(tmp != NULL && i<input_count)
    {
        if(tmp->type==INPUT)
        {
            /*convert char to int and assign to output
            * of the INPUT gate */
            tmp->nOutput=line[i]-'0';
            i++;
        }
        tmp=tmp->nextGate;
    }

    return 0;
}

/*
* Function:  calculate_gate
* --------------------------
* calculates the output of the gate by gatetype
* and input values
*
*  src: gate to be calculated
*
*  returns: 0
*/

int calculate_gate(struct gate * src)
{
    int res=-1;

    switch (src->type)
    {
    case AND:
    {
        /* do and operation on all inputs
         * and assign the result to output */
        for(int i=0; i<src->nInputs; i++)
        {
            if (res==-1)
                res=src->inputs[i]->nOutput;
            else
                res &= src->inputs[i]->nOutput;
        }
        src->nOutput=res;
        break;
    }
    case OR:
    {
        /* do or operation on all inputs
         * and assign the result to output */
        for(int i=0; i<src->nInputs; i++)
        {
            if (res==-1)
                res=src->inputs[i]->nOutput;
            else
                res |= src->inputs[i]->nOutput;
        }
        src->nOutput=res;
        break;
    }
    case NOT:
        src->nOutput=!src->inputs[0]->nOutput;
        break;

    case OUTPUT:
        src->nOutput = src->inputs[0]->nOutput;
        break;

    case FLIPFLOP:
        if(src->inputs[0]->nOutput == 0 && src->former_out == 0)
        {
            src->nOutput = 0;
        }
        else if(src->inputs[0]->nOutput == 0 && src->former_out == 1)
        {
            src->nOutput = 1;
        }
        else if(src->inputs[0]->nOutput == 1 && src->former_out == 0)
        {
            src->nOutput = 1;
            src->former_out = 1;
        }
        else if(src->inputs[0]->nOutput == 1 && src->former_out == 1)
        {
            src->nOutput = 0;
            src->former_out = 0;
        }
        break;
    default:
        break;
    }
    return 0;
}

/*
* Function:  evaluate_circuit
* --------------------------
* calculate all circuit gate outputs
*
*  returns: 0
*/

int evaluate_circuit()
{
    struct gate * g=head;
    while(g != NULL)
    {
        if (g->nOutput == -1) // check if gate has output
        {
            int all_inputs_ok = true;
            // check if all inputs have output value
            for(int i=0; i<g->nInputs; i++)
            {
                if (g->inputs[i]->nOutput == -1)
                    all_inputs_ok = false;
            }
            // if all inputs have value, calculate the output
            if(all_inputs_ok)
                calculate_gate(g);
        }
        g=g->nextGate;
    }
    return 0;
}

/*
* Function:  add_connection_to_circuit
* ------------------------------------
* add connection to circuit
*
*  data: source and destination gate names
*
*  returns: 0
*/

int add_connection_to_circuit(char ** data)
{
    struct gate * src;
    struct gate * dst;

    src=find_gate_by_name(data[1]);
    dst=find_gate_by_name(data[2]);
    // add destination to the output gate of source
    src->output=dst;
    /* reallocate memory for destination gate inputs
     * and add the source gate */
    dst->nInputs++;
    int n = dst->nInputs;
    dst->inputs = realloc(dst->inputs, n*sizeof(struct gate));
    dst->inputs[n-1] = src;

    return 0;
}

/*
* Function:  add_gate_to_circuit
* -------------------------------
*  add gate to circuit
*
*  data: type and name of the gate
*
*  returns: 0
*/

int add_gate_to_circuit(char ** data)
{
    // allocate space for the gate in memory
    struct gate * newGate=(struct gate *) malloc(sizeof(struct gate));
    // assign default and data values to gate
    newGate->former_out = 0;
    newGate->inputs = (struct gate **)malloc(sizeof(struct gate));
    newGate->nInputs = 0;
    strcpy((char*)newGate->name, data[2]);
    newGate->nOutput=-1;
    newGate->type = str_to_enum(data[1]);
    newGate->nextGate=NULL;
    newGate->output=NULL;

    /* if head exists add gate to linked list
     * else head is this gate*/
    if(head)
    {
        current->nextGate=newGate;
        current=newGate;
    }
    else
    {
        head=current=newGate;
    }

    return 0;
}

/*
* Function:  format_circuit_line
* --------------------------------
* format the circuit text line for use
*
*  line: read from the circuit text file
*
*  returns: char array of fields
*/

char ** format_circuit_line(char line[])
{
    line[strcspn(line, "\n")] = 0;   // remove newline
    char delim[] = " ";              // define line delimiter
    /* allocate memory space for char array */
    int word_count = 3;
    char ** word = malloc(word_count * sizeof(char*));
    for (int i =0 ; i < word_count; ++i)
        word[i] = malloc(20 * sizeof(char));
    /* split the line into tokens */
    int i = 0;
    char * temp;
    temp=strtok(line, delim);
    while(temp != NULL)
    {
        word[i]=temp;
        temp = strtok(NULL, delim);
        i++;
    }
    return word;
}

/*
* Function:  file_exists
* --------------------------------
* check if file  exists
*
*  fp: file pointer
*
*  returns: 1 if exists else 0
*/

int file_exists ( FILE * fp)
{
    if (fp)
    {
        return true;
    }
    return false;
}

/*
* Function:  open_file
* --------------------------------
* open file for read operation
*
*  file_name: name of the file
*
*  returns: file pointer that is opened
*/

FILE * open_file (char * file_name)
{
    /* define file pointer and assign it to opened file */
    FILE * fp = fopen(file_name, "r");
    if (!file_exists(fp))                   // check if file exists
    {
        printf("%s not found!", file_name);
        exit(EXIT_FAILURE);
    }
    return fp;
}

/*
* Function:  generate_circuit
* --------------------------------
* start reading circuit text file
*
*  returns: 0
*/

int generate_circuit()
{
    char line[MAX_STRING];                      /* define line max size to be read */
    FILE * fp;

    fp = open_file((char*)CIRCUIT_FILE);        /* open file */
    while(fgets(line,MAX_STRING,fp) != NULL)    /* read each line until EOF */
    {
        char ** gate_data=format_circuit_line(line);
        if ( strcmp(gate_data[0],GATE) == 0)
        {
            add_gate_to_circuit(gate_data);
        }
        else
        {
            add_connection_to_circuit(gate_data);
        }
    }

    fclose(fp);
    return 0;
}

/*
* Function:  reset_circuit
* --------------------------------
* clear all gates output values
* keep former_out for FLIPFLOP gates
*
*  returns: 0
*/

int reset_circuit()
{
    struct gate * tmp=head;
    while(tmp != NULL)
    {
        tmp->nOutput = -1;
        tmp=tmp->nextGate;
    }
    return 0;
}

/*
* Function:  get_input_values
* --------------------------------
* read input file
*
*  returns: 0
*/

int get_input_values()
{
    char line[MAX_STRING];
    FILE * fp = open_file((char*)INPUT_FILE);

    while(fgets(line,MAX_STRING,fp) != NULL)    /* read each line until EOF */
    {
        assign_input_values(line);
        evaluate_circuit();

        struct gate * result=head;
        while(result != NULL)
        {
            if(result->type == OUTPUT)
                printf("%d\n",result->nOutput);
            result=result->nextGate;
        }

        reset_circuit();
    }

    fclose(fp);
    return 0;
}

int main()
{
    generate_circuit();
    get_input_values();



    return 0;
}




