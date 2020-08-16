//I think this is supposed to be a multi-dimensional array, but I haven't
//been able to reverse-engineer the meaning of the fields yet.
//I don't think this works if you have more than three array dimensions?
typedef struct _a{
    long t;    //??? Has something do do with boxes
    long r;    //rank, number of valid dimensions in the d array
    long d[3]; //dimensions
    long p[];  //I took out the [2] (which depends on C99) to help imply that 
               //that these structs are manually allocated with the right
               //size for all the array's data members (see the ga function)
} *A;

//The 'a' and 'w' are supposed to mean alpha and omega, as in the original
//APL syntax 
#define V1(f) A f(w)A w;
#define V2(f) A f(a,w)A a,w;

//Allocate space of n longs. (ma = "Make A"?)
//Corrected: used to say malloc(n*4), but on my computer a long is 8 bytes
long *ma(int n) {
    return (long*) malloc(n * sizeof(long));
}

//Copy n longs from source to dest. I'm half-tempted to just replace this
//with calls to memcpy but whatever. I think "mv" is a pretty bad name,
//because this function does NOT work if the ranges alias each other
void mv(long *d, long *s, int n) {
    int i;
    for (i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

//Takes r elements from array d and returns their product. The r must stand
//for "rank" and the d for "dimensions". I'm guessing it's called tr stands
//for "trace".
void tr(int r, long *d) {
    long z = 1;
    int i;
    for (i = 0; i < r; i++) {
        z *= d[i];
    }
    return z;
}

//Constructs a multidimensional array with the given rank and dimensions.
//I'm guessing "ga" stands for "Get Array"
A ga(int t, int r, long *d) {
    //Here, the (5+tr(r,d)) makes space for the 5 longs at the start of the
    //multidim array struct (i.e. fields t, r, and d) and then enough space
    //for however many longs are needed to store the array data
    A z = (A) ma(5+tr(r,d));
    
    z->t = t; //Something to do with boxes?
    z->r = r; //Provide the rank
    mv(z->d, d, r); //Provide the dimensions
    
    return z;
}


A iota (A w) {
    long n = *w->p;   //Assumes w is rank 0. 
    A z = ga(0,1,&n); //Make a rank 1 array...
    int i;
    for (i = 0; i < n; i++) {
        z->p[i] = i;  //...and fill it with an increasing count
    }
    return z;
}

A plus(A a, A w) {
    long r=w->r, *d=w->d, n=tr(r,d); //Rank, dimension, and number of elements
    A z = ga(0,r,d);
    
    int i;
    for (i = 0; i < n; i++) {
        z->p[i] = a->p[i] + w->p[i];
    }
    return z;
}

V2(from){long r=w->r-1,*d=w->d+1,n=tr(r,d);
 A z=ga(w->t,r,d);mv(z->p,w->p+(n**a->p),n);return z;}
V1(box){A z=ga(1,0,0);*z->p=(long)w;return z;}
V2(cat){long an=tr(a->r,a->d),wn=tr(w->r,w->d),n=an+wn;
 A z=ga(w->t,1,&n);mv(z->p,a->p,an);mv(z->p+an,w->p,wn);return z;}
V2(find){}
V2(rsh){long r=a->r?*a->d:1,n=tr(r,a->p),wn=tr(w->r,w->d);
 A z=ga(w->t,r,a->p);mv(z->p,w->p,wn=n>wn?wn:n);
 if(n-=wn)mv(z->p+wn,z->p,n);return z;}
V1(sha){A z=ga(0,1,&w->r);mv(z->p,w->d,w->r);return z;}
V1(id){return w;}V1(size){A z=ga(0,0,0);*z->p=w->r?*w->d:1;return z;}
pi(i){printf("%d ",i);}nl(){printf("\n");}
pr(w)A w;{long r=w->r,*d=w->d,n=tr(r,d);DO(r,pi(d[i]));nl();
 if(w->t)DO(n,printf("< ");pr(w->p[i]))else DO(n,pi(w->p[i]));nl();}

char vt[]="+{~<#,";
A(*vd[])()={0,plus,from,find,0,rsh,cat},
 (*vm[])()={0,id,size,iota,box,sha,0};
long st[26]; qp(a){return  a>='a'&&a<='z';}qv(a){return a<'a';}
A ex(e)long *e;{long a=*e;
 if(qp(a)){if(e[1]=='=')return st[a-'a']=ex(e+2);a= st[ a-'a'];}
 return qv(a)?(*vm[a])(ex(e+1)):e[1]?(*vd[e[1]])(a,ex(e+2)):(A)a;}
noun(c){A z;if(c<'0'||c>'9')return 0;z=ga(0,0,0);*z->p=c-'0';return z;}
verb(c){long i=0;for(;vt[i];)if(vt[i++]==c)return i;return 0;}
long *wd(char *s){long a,n=strlen(s),*e=ma(n+1);char c;
 DO(n,e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);e[n]=0;return e;}

main(){
    char s[99];
    while(gets(s))
        pr(ex(wd(s)));
}
