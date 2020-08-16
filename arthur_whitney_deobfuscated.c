//Includes to get rid of warnings
#include <stdio.h>
#include <stdint.h> //For intptr_t

//BAD BAD BAD: this code has memory leaks from here til second coming!

//So... why have I done this?
//It's because the original implementation depends on numbers and pointers
//having the same size. Check out the multidimensional array struct (below):
//its p[] array can either store numbers (for regular arrays) or pointers
//(for arrays of boxes). I kept the original "I" typedef that Arthur Whitney
//used originally.
typedef I intptr_t;

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
void tr(int r, I *d) {
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
    
    A z = ga(1, &n); //Create a rank-1 array with n elements
    
    mv(z->p,a->p,an);       //Copy in a's elements
    mv(z->p+an,w->p,wn);    //Copy in w's elements
    
    return z;
}

//I guess this was never implemented?
//V2(find){}

//Reshape?
A rsh(A a, A w) {
    //Changed *a->d to a->d[0] for clarity
    I r = a->r ? a->d[0] : 1; //
    I n = tr(r, a->p);  //
    I wn=tr(w->t, w->r, w->d);
    
    A z=ga(w->t,r,a->p);mv(z->p,w->p,wn=n>wn?wn:n);
    if(n-=wn)mv(z->p+wn,z->p,n);
    return z;
}

//Shape of?
V1(sha){A z=ga(0,1,&w->r);mv(z->p,w->d,w->r);return z;}


V1(id){return w;}V1(size){A z=ga(0,0,0);*z->p=w->r?*w->d:1;return z;}

//Print an int
pi(i){printf("%l ",i);}nl(){printf("\n");}

//Print a boxed item
pr(w)A w;{I r=w->r,*d=w->d,n=tr(r,d);DO(r,pi(d[i]));nl();
 if(w->t)DO(n,printf("< ");pr(w->p[i]))else DO(n,pi(w->p[i]));nl();}

char vt[]="+{~<#,";
A(*vd[])()={0,plus,from,find,0,rsh,cat},
 (*vm[])()={0,id,size,iota,box,sha,0};
I st[26]; qp(a){return  a>='a'&&a<='z';}qv(a){return a<'a';}
A ex(e)I *e;{I a=*e;
 if(qp(a)){if(e[1]=='=')return st[a-'a']=ex(e+2);a= st[ a-'a'];}
 return qv(a)?(*vm[a])(ex(e+1)):e[1]?(*vd[e[1]])(a,ex(e+2)):(A)a;}
noun(c){A z;if(c<'0'||c>'9')return 0;z=ga(0,0,0);*z->p=c-'0';return z;}
verb(c){I i=0;for(;vt[i];)if(vt[i++]==c)return i;return 0;}
I *wd(char *s){I a,n=strlen(s),*e=ma(n+1);char c;
 DO(n,e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);e[n]=0;return e;}

main(){
    char s[99];
    while(gets(s))
        pr(ex(wd(s)));
}
