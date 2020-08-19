using System;
using System.Collections;
using System.Collections.Generic;

//Part of the reason I'm using C# is specifically to learn proper techniques
//for runtime plymorphism. Parsing and ASTs is as good a use case as any
public class jword {
    //I guess my code is just going to be verbose. Oh well
    public enum PartOfSpeech {
        Noun,
        Verb,
        Adverb,
        Conjunction,
        LeftParen,
        RightParen,
        Assignment,
        Sentinel
    }
    
    //Right now I'm using properties because it is apparently "good 
    //practice". However, I'm hoping that by making my types immutable (and
    //I hope I'm doing it properly) I can reap the benefits of optimization
    public PartOfSpeech WordType{get; private set;}
    
    public jword(PartOfSpeech type) {
        this.WordType = type;
    }
    
    private static Dictionary<string, int> verbDict;
    private static Dictionary<string, int> adverbDict;
    private static Dictionary<string, int> conjunctionDict;
    
    static jword() {
        //TODO: put in aaaaaallllll of the operators
        //For now, let's just put in a few test ones
        verbDict["+"] = 0;
        verbDict["*"] = 1;
        verbDict["i."] = 2;
        
        adverbDict["/"] = 0;
        
        conjunctionDict["&"] = 0;
    }
    
}

public class jnoun : jword {
    public enum NounType {
        Integer, //All these types are multidimensional arrays, except for Name
        Float,
        Rational,
        String,
        Box,
        Name
    }
    
    public NounType Type{get; private set;}
    
    public jnoun(NounType type) : base(jword.PartOfSpeech.Noun) {
        this.Type = type;
    }
}

//I didn't want to write a class for every type... that's what generics are
//for. However, now the constructor needs a parameter to say "what type am
//I", when technically this isn't necessary. However, I can't see how to get
//around this issue.
public class jarray<T> : jnoun {
    //I'm not really sure using properties is worth the extra trouble here
    public int[] Dimensions{get; private set;}
    public T[] Data{get; private set;}
    
    //Yeah, I would love some dumplings right about now
    private static int dimsum(int[] dims) {
        int ret = 1;
        if (dims != null) {
            for (int i = 0; i < dims.Length; i++) {
                ret *= dims[i];
            }
        }
        return ret;
    }
    
    //Since rank 0 arrays are probably going to be pretty common, I'd like
    //to make it possible to use null for dims instead of a length-0 array
    public jarray(jnoun.NounType type, int[] dims) : base(type) {
        //Now is the time to check for errors
        if (type == jnoun.NounType.Name) {
            throw new ArgumentException("Arrays of Names are currently not allowed", "type");
        }
        
        if (dims == null) Dimensions = dims;
        else Dimensions = (int[]) dims.Clone(); //Cloning keeps API simple
        
        Data = new T[dimsum(dims)];
    }
    
    //This constructs a jarray from an existing array but the given
    //dimensions. Clone works in the same way as before
    public jarray(jnoun.NounType type, int[] dims, T[] data, bool clone = true) : base(type) {
        //Now is the time to check for errors
        if (type == jnoun.NounType.Name) {
            throw new ArgumentException("Arrays of Names are currently not allowed", "type");
        }
        
        //Check that given dimensions make sense
        if (dimsum(dims) != data.Length) {
            throw new ArgumentException("Given dimensions must fit given initializer array", "dims and data");
        }
        
        //Clone the input data if requested
        if (clone) this.Data = (T[]) data.Clone();
        else this.Data = data;
        
        //Dimensions are always cloned. It's just simpler and has tiny performance impact
        this.Dimensions = (int[]) dims.Clone();
    }    
    
    //I used U to avoid name collision with 'T'
    private static int[] get_dims<U> (U[] data) {
        if (data == null) {
            throw new ArgumentNullException("Cannot currently construct a jarray from a null array", "data");
        }
        
        return new int[1] {data.Length};
    }
    
    //This constructs a jarray from an existing array. The rank is assumed 
    //to be 1, and the dimension will match the given array. The clone 
    //boolean causes the input array to be cloned (true by default)
    public jarray(jnoun.NounType type, T[] data, bool clone = true)
     : this(type, get_dims(data), data, clone) 
    {}
}

//For holding a variable name
public class jname : jnoun {
    public string Name{get; private set;}
    
    public jname(string name) : base(jnoun.NounType.Name) {
        this.Name = name;
    }
}

public class jverb : jword {    
    public int VerbIndex {get; private set;}
    
    public jverb(int idx) : base(jword.PartOfSpeech.Verb) {
        this.VerbIndex = idx;
    }
}

public class jadverb : jword {    
    public int AdverbIndex {get; private set;}
    
    public jadverb(int idx) : base(jword.PartOfSpeech.Adverb) {
        this.AdverbIndex = idx;
    }
}

public class jconj : jword {    
    public int ConjIndex {get; private set;}
    
    public jconj(int idx) : base(jword.PartOfSpeech.Conjunction) {
        this.ConjIndex = idx;
    }
}
