//Includes to get rid of warnings
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> //For intptr_t
#include <inttypes.h> //For printf format strings
#include <string.h>

//Change this to the right printf specifier depending on the size of an 
//intptr_t on your machine
#define NUM_FORMAT "%d"

//BAD BAD BAD: this code has memory leaks from here til second coming!

//So... why have I done this?
//It's because the original implementation depends on numbers and pointers
//having the same size. Check out the multidimensional array struct (below):
//its p[] array can either store numbers (for regular arrays) or pointers
//(for arrays of boxes). I kept the original "I" typedef that Arthur Whitney
//used originally.
typedef intptr_t I;

//I think this is supposed to be a multi-dimensional array, but I haven't
//been able to reverse-engineer the meaning of the fields yet.
//I don't think this works if you have more than three array dimensions?
typedef struct _a{
    I t;    //Whether this is an array of boxes. Normally a single char
            //would be enough to hold this information, but Whitney's code
            //depends on this flag having the same byte size as everything 
            //else.
    I r;    //Rank, number of valid dimensions in the d array
    I d[3]; //Dimensions
    I p[];  //I took out the [2] (which depends on C99) to help imply that 
            //that these structs are manually allocated with the right
            //size for all the array's data members (see the ga function)
            //By the way, this member is occasionally casted to "array
            //of pointers to struct _a" if t = 1.
            //In the original implementation, Whitney never makes use of
            //the fact that he put a 2 here. He probably only did that
            //because his compiler did not support the modern syntax.
} *A;

//The 'a' and 'w' are supposed to mean alpha and omega, as in the original
//APL syntax 
#define V1(f) A f(w)A w;
#define V2(f) A f(a,w)A a,w;

//Allocate space of n Is. (ma = "Make A"?)
//Corrected: used to say malloc(n*4), but on my computer an I is 8 bytes
I *ma(int n) {
    return (I*) malloc(n * sizeof(I));
}

//Copy n numbers from source to dest. I'm half-tempted to just replace this
//with calls to memcpy but whatever. I think "mv" is a pretty bad name,
//because this function does NOT work if the ranges alias each other
void mv(I *d, I *s, int n) {
    int i;
    for (i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

//Takes r elements from array d and returns their product. The r must stand
//for "rank" and the d for "dimensions". I'm guessing it's called tr stands
//for "trace".
int tr(int r, I *d) {
    I z = 1;
    int i;
    
    for (i = 0; i < r; i++) {
        z *= d[i];
    }
    
    return z;
}

//Constructs a multidimensional array with the given rank and dimensions.
//I'm guessing "ga" stands for "Get Array"
A ga(int t, int r, I *d) {
    //Here, the (5+tr(r,d)) makes space for the 5 fields at the start of the
    //multidim array struct (i.e. t, r, and d[3]) and then enough space
    //for however many Is are needed to store the array data.
    A z = (A) ma(5+tr(r,d)); 
    
    z->t = t; //Something to do with boxes?
    z->r = r; //Provide the rank
    mv(z->d, d, r); //Provide the dimensions
    
    return z;
}

//Same as iota from APL.
A iota (A w) {
    I n = *w->p;   //Assumes w is rank 0. 
    A z = ga(0,1,&n); //Make a rank 1 array...
    I i;
    for (i = 0; i < n; i++) {
        z->p[i] = i;  //...and fill it with an increasing count
    }
    return z;
}

//Does not check that the arrays are not boxed arrays. Does not check that
//dimensions match. Returns the sum of the two arrays.
A plus(A a, A w) {
    I r=w->r, *d=w->d, n=tr(r,d); //Rank, dimension, and number of elements
    A z = ga(0,r,d);
    
    int i;
    for (i = 0; i < n; i++) {
        z->p[i] = a->p[i] + w->p[i];
    }
    return z;
}

//Does not check boxed vs. unboxed
//Assumes a is rank 0. This returns w(a->p[0]), which has one less dimension
//than w originally had.
A from(A a, A w) {
    //I think "cell" has a particular meaning in J, but ignore that for now.
    //Basically, w is a multidimensional array, which can also be thought
    //of as a rank 1 array of cells (where each cell is could also be a 
    //multidimensional array):
    //
    // w = {
    //      cell_0,
    //      cell_1,
    //      ...
    //      cell_n
    // }
    //
    // For example, if w had dimensions {4,5,6}, then you could imagine w
    // was an array of four cells, where each cell is an array of
    // dimension {5,6}
    //
    // This function uses the number in a to index into the cells of w, and
    // returns the result.
    
    I r  = w->r-1;  
    I *d = w->d+1;  //Drops the first dimension
    I n  = tr(r,d); //Number of elements in one cell
    
    A z = ga(w->t,r,d); 
    
    //Copy the chunk indicated by a into the result array
    mv(z->p, w->p + (n * a->p[0]), n); //I changed *a->p to a->p[0] for clarity
    
    return z;
}

//Enclose the given array in a box. The returned value is actually another
//array, but with its t field set to 1 (to indicate that it is a box) and 
//its p[] array containing a pointer to the original array.
A box(A w) {
    A z = ga(1, 0, 0);
    //That is so hideous... the p[] array in the A structs can also be a 
    //pointer to another A struct, and that's how you do boxes...
    
    //Original:    z->p[0] = (I)w;
    //Fixed for portability:
    *((A*) z->p) = w;
    
    return z;
}

//Ravel (i.e. treat as a linear array) and concatenate two arrays. This can
//also concatenate arrays of boxes, but doesn't to check to make sure that
//both are boxed
A cat(A a, A w) {
    I an = tr(a->r, a->d); //Number of elements in a
    I wn = tr(w->r, w->d); //Number of elements in w
    I n  = an + wn; //Number of elements in the result
    
    A z = ga(w->t, 1, &n); //Create a rank-1 array with n elements
    
    mv(z->p,a->p,an);       //Copy in a's elements
    mv(z->p+an,w->p,wn);    //Copy in w's elements
    
    return z;
}

//I guess this was never implemented?
//V2(find){}
A find(A w) {
    return w; //Tries to do something reasonable
}

//Reshape
A rsh(A a, A w) {
    //Changed *a->d to a->d[0] for clarity
    //The idea is to use a->p[] as the new dimensions.
    //This code assumes a is rank-1 or rank-0
    I r = a->r ? a->d[0] : 1; //New rank. This is either a->d[0] if a has
                              //rank 1, or 1 if a has rank 0.
    I n = tr(r, a->p); //New dimensions.
    I wn= tr(w->r, w->d); //Number of elements in w. We need to know this
                          //because resphape will replicate w's members
                          //if the new size is bigger
    
    A z = ga(w->t,r,a->p); //Allocate new multidim array
    
    //The original implementation was wrong. I have rewritten it.
    int offset = 0; //Current offset into the destination vector
    while (offset < n) { 
        //Loop until we've filled the output with copies of w->p[]                
        int num_to_transfer = (n > wn) ? wn : n; // = min(wn, n), make sure
                                                 // that we don'tread past
                                                 // the end of w->p[]
        mv(z->p + offset, w->p, num_to_transfer);
        offset += num_to_transfer;
    }
    
    //Original:
    // mv(z->p,w->p,wn=n>wn?wn:n); 
    // if(n-=wn)mv(z->p+wn,z->p,n);
        
    return z;
}

//Returns the shape of w
A sha(A w) {
    //Fairly simple, just constructs a new array whose p array is the 
    //dimensions of w
    A z = ga(0,1,&w->r); //The result is a rank-1 array of numbers
    mv(z->p,w->d,w->r); //Copy w->d[] into the result's data array
    
    return z;
}

//Identity function
A id(A w) {
    return w;
}

//Returns number of cells in w (according to my own definition of a "cell",
//see the comments in the "from" function)
A size (A w) {
    A z = ga(0,0,0); //Make a rank-0 array of numbers
    
    //I changed (*w->d) to (w->d[0]) for clarity
    *z->p = (w->r) ? w->d[0] : 1; //If w has rank 0, then we would say it has 1 cell
    
    return z;
}

//Print an int
void pi(I i) {
    printf(NUM_FORMAT " ", i);
}

//Print a newline    
void nl(void) {
    printf("\n");
    fflush(stdout);
}

//Print a multidim array
void pr(A w){
    I r=w->r,*d=w->d,n=tr(r,d); //Rank, dimensions, and number of elements
    
    //First print out the dimensions
    int i;
    for (i = 0; i < r; i++) {
        pi(d[i]);
    } 
    nl();
    
    if(w->t) {
        //Print array of boxed items
        int i;
        for (i = 0; i < n; i++) {
            printf("< ");
            pr((A) w->p[i]);
        }
    } else {
        //Print the linearized elements
        int i;
        for (i = 0; i < n; i++) {
            pi(w->p[i]);
        } 
        nl();
    }
}

char vt[]="+{~<#,"; //Operators

//Dispatch table for dyadic operators
A(*vd[])() = {
    //0,  //Original used 1-indexing for some reason
    plus, //'+'
    from, //'{'
    find, //'~'
    0,    //'<', does not have dyadic version
    rsh,  //'#'
    cat   //','
};

//Dispatch table for monadic operators
A(*vm[])() = {
    //0,  //Original used 1-indexing for some reason
    id,   //'+'
    size, //'{'
    iota, //'~'
    box,  //'<'
    sha,  //'#'
    0     //',', does not have monadic version
};

A st[26]; //Storage registers, one for each lowercase letter

//Says if char is a lowercase letter (and thus is a variable name)
//I have no idea why this is called 'qp'
int qp(I a) {
    return  a>='a' && a<='z';
}

//Says if char is less than 'a'. In the array of tokens, anything less than
//'a' is an operator. (Nouns are pointers to multidim arrays, so they are
//bigger than 'a', and identifiers are from 'a' to 'z')
//I don't know why on Earth it's called 'qv'
int qv(I a) {
    return a<'a';
}

//Parse and evaluate an expression. The input is an array of tokens. Each
//token is either a noun (which is a pointer to a multidim array of rank 0)
//a verb (an integer from 0 to 5 inclusive), or a raw ASCII code for variable
//names or the equals operator.
//Whitespace is not tolerated anywhere in any expressions! This means that 
//to type out an array, you have to put a bunch of concat operators (',')
//and painfully copy all that memory. Not efficient!
A ex(I *e) {
    I a = e[0];
    
    //Check if a is an identifier
    if(qp(a)) {
        //If so, check if this is an assignment expressions. Actually, 
        //because this function returns the LHS, you can chain assignment
        //with other operations.
        if(e[1] == '=') {
            return st[a-'a'] = ex(e+2); //Recursively parse RHS
        } else {
            a = (I) st[a-'a']; //Fetch a variable
        }
    }
    
    if (qv(a)) {
        //This means a is an operator, and must be monadic. So, run the 
        //appropriate function on the rest of the expression (which is
        //recursively parsed)
        return (*vm[a])(ex(e + 1));
    } else if (e[1] != -1) {
        //If there is more than token left, and because the first token 
        //wasn't an operator, this must be a dyadic expression. (Of course,
        //like the rest of this code, nothing is checked for invalid input
        //or errors)
        return (*vd[e[1]])(a, ex(e + 2));
    } else {
        //This means it must be a noun. Return it
        return (A) a;
    }
    
    //This was the most difficult thing to deobfuscate!
    //return qv(a)?(*vm[a])(ex(e+1)):e[1]?(*vd[e[1]])(a,ex(e+2)):(A)a;
}

//Parse a noun (i.e. a literal number)
//I guess this only supports numbers from 0 to 9 (inclusive). Returns NULL
//if this is not a noun
A noun(char c){
    A z;
    if(c<'0'||c>'9') return NULL;
    z=ga(0,0,0); //Rank-zero array of numbers
    z->p[0] = c-'0'; //"Parse" the integer
    return z;
}

//Parse a verb (i.e. a single-character operator from "+{~<#,"
//Returns -1 to say "this is not a verb" 
//(note: the original returned zero but I thought that didn't make sense)
I verb(char c) {
    int i;
    for(i = 0; vt[i]; i++) {
        if(vt[i]==c) return i; //I moved the increment to its natural place
                               //so that the arrays are zero-indexed
    }
    return -1;
}

I* wd(char *s){
    I a;
    I *e = ma(strlen(s)+1);   //Array of tokens with space for sentinel
    char c;
    int i;
    for (i = 0; s[i]; i++) {
        c = s[i];
        //More casting between pointers and numbers. Why not use a union?
        //(This was originall written in the 80s; maybe unions didn't yet
        //exist?)
        if ((A)(a = (I) noun(c)) != NULL) {
            e[i] = a;
        } else if ((a = verb(c)) >= 0) {
            e[i] = a;
        } else {
            e[i] = c;
        }
        
        //This was the second most difficult thing to deobfuscate!
        //e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c
    }
    e[i] = -1; //Sentinel
    return e; 
}

int main(){
    char s[99];
    while(gets(s))
        pr(ex(wd(s)));
    
    return 0;
}
