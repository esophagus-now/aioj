using System;
using System.Collections;
using System.Collections.Generic;

public class jparser {
    private IEnumerator<jlexer.Token> tokens;
    
    //This is basically a shift-reduce parser with a lookahead of 4 tokens
}

public class AIOJ {    
    public static void Main(string[] args) {
        Console.WriteLine("Hello world");
        
        var j = new jlexer("sum =.+/_6.95*i.3 4\n");
        foreach (var tok in j) {
            Console.WriteLine(tok);
        }
    }
}
