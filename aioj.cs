using System;
using System.Collections;
using System.Collections.Generic;

public class jlexer {    
    private string text;
    
    public jlexer(string text) {
        this.text = text;
    }
    
    private enum charclass {
        Blank = 0,
        Other,
        Letter,
        N,
        B,
        Numeric,
        Period,
        Colon,
        Quote
    }
    
    private charclass classify (char c) {
        if (char.IsLetter(c)) {
            if (c == 'N') return charclass.N;
            else if (c == 'B') return charclass.B;
            else return charclass.Letter;
        } else if (char.IsDigit(c) || c == '_') {
            return charclass.Numeric;
        } else if (char.IsWhiteSpace(c)) {
            return charclass.Blank;
        } else if (c == '.') {
            return charclass.Period;
        } else if (c == ':') {
            return charclass.Colon;
        } else if (c == '\'') {
            return charclass.Quote;
        } else {
            return charclass.Other;
        }
    }
    
    public enum lexstate {
        Blank = 0,
        Other,
        Name,
        N,
        NB,
        NBPeriod,
        Number,
        Quote,
        AdjacentQuotes,
        Comment
    }
    
    private enum lexaction {
        Nothing,
        StartOfToken,
        EndOfToken
    }
    
    private struct Transition {
        private readonly lexstate nextState;
        private readonly lexaction action;
        
        public Transition (lexstate nextState, lexaction action) {
            this.nextState = nextState;
            this.action = action;
        }
        
        public lexstate NextState {get {return nextState;}}
        public lexaction Action {get {return action;}}
        
        public override string ToString() {
            return "[next: " + nextState + ", action: " + action + "]";
        }
    }
    
    private static readonly Transition[,] transitions = {
    //States    // Blank                                                  Other                                                       Letter                                                      N                                                           B                                                           Numeric                                                     Period                                                      Colon                                                       Quote
    /*Blank     */{new Transition(lexstate.Blank, lexaction.Nothing),     new Transition(lexstate.Other, lexaction.StartOfToken),     new Transition(lexstate.Name, lexaction.StartOfToken),      new Transition(lexstate.N, lexaction.StartOfToken),         new Transition(lexstate.Name, lexaction.StartOfToken),      new Transition(lexstate.Number, lexaction.StartOfToken),    new Transition(lexstate.Other, lexaction.StartOfToken),     new Transition(lexstate.Other, lexaction.StartOfToken),     new Transition(lexstate.Quote, lexaction.StartOfToken)},
    /*Other     */{new Transition(lexstate.Blank, lexaction.EndOfToken),  new Transition(lexstate.Other, lexaction.EndOfToken),       new Transition(lexstate.Name, lexaction.EndOfToken),        new Transition(lexstate.N, lexaction.EndOfToken),           new Transition(lexstate.Name, lexaction.EndOfToken),        new Transition(lexstate.Number, lexaction.EndOfToken),      new Transition(lexstate.Other, lexaction.Nothing),          new Transition(lexstate.Other, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.EndOfToken)},
    /*Name      */{new Transition(lexstate.Blank, lexaction.EndOfToken),  new Transition(lexstate.Other, lexaction.EndOfToken),       new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.Other, lexaction.Nothing),          new Transition(lexstate.Other, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.EndOfToken)},
    /*N         */{new Transition(lexstate.Blank, lexaction.EndOfToken),  new Transition(lexstate.Other, lexaction.EndOfToken),       new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.NB, lexaction.Nothing),             new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.Other, lexaction.Nothing),          new Transition(lexstate.Other, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.EndOfToken)},
    /*B         */{new Transition(lexstate.Blank, lexaction.EndOfToken),  new Transition(lexstate.Other, lexaction.EndOfToken),       new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.Name, lexaction.Nothing),           new Transition(lexstate.NBPeriod, lexaction.Nothing),       new Transition(lexstate.Other, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.EndOfToken)},
    /*NBPeriod  */{new Transition(lexstate.Comment, lexaction.Nothing),   new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Other, lexaction.Nothing),          new Transition(lexstate.Other, lexaction.Nothing),          new Transition(lexstate.Comment, lexaction.Nothing)},
    /*Number    */{new Transition(lexstate.Blank, lexaction.EndOfToken),  new Transition(lexstate.Other, lexaction.EndOfToken),       new Transition(lexstate.Number, lexaction.Nothing),         new Transition(lexstate.Number, lexaction.Nothing),         new Transition(lexstate.Number, lexaction.Nothing),         new Transition(lexstate.Number, lexaction.Nothing),         new Transition(lexstate.Number, lexaction.Nothing),         new Transition(lexstate.Other, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.EndOfToken)},
    /*Quote     */{new Transition(lexstate.Quote, lexaction.Nothing),     new Transition(lexstate.Quote, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.Nothing),          new Transition(lexstate.Quote, lexaction.Nothing),          new Transition(lexstate.AdjacentQuotes, lexaction.Nothing)},
    /*AdjQuotes */{new Transition(lexstate.Blank, lexaction.EndOfToken),  new Transition(lexstate.Other, lexaction.EndOfToken),       new Transition(lexstate.Name, lexaction.EndOfToken),        new Transition(lexstate.N, lexaction.EndOfToken),           new Transition(lexstate.Name, lexaction.EndOfToken),        new Transition(lexstate.Number, lexaction.EndOfToken),      new Transition(lexstate.Other, lexaction.EndOfToken),       new Transition(lexstate.Other, lexaction.EndOfToken),       new Transition(lexstate.Quote, lexaction.Nothing)},
    /*Comment   */{new Transition(lexstate.Comment, lexaction.Nothing),   new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing),        new Transition(lexstate.Comment, lexaction.Nothing)}
    };
    
    public struct Token {
        public enum toktype {
            Name = 0,
            Number,
            Quote,
            Other
        }
        public toktype Type {get; private set;}
        public string Text {get; private set;}
        
        public Token(lexstate state, string text) : this() {
            switch(state) {
            case lexstate.Name:
                Type = toktype.Name;
                break;
            case lexstate.Number:
                Type = toktype.Number;
                break;
            case lexstate.Quote:
                Type = toktype.Quote;
                break;
            default:
                Type = toktype.Other;
                break;
            }
            Text = text;
        }
        
        public override string ToString() {
            return "[" + Type + ": \"" + Text + "\"]";
        }
    }
    
    // Iterates through text and pulls out tokens
    public IEnumerator<Token> GetEnumerator() {
        lexstate state = lexstate.Blank;
        int tokstart = 0;
        for (int pos = 0; pos < text.Length; pos++) {
            charclass cls = classify(text[pos]);
            Transition t = transitions[(int)state,(int)cls];
            //Console.Write(text[pos] + " || state = " + state + ", in = " + cls + "\t=>\t" + t.Action + "\n");
            switch(t.Action) {
            case lexaction.StartOfToken:
                //This is the '=' action from aioj
                tokstart = pos;
                state = t.NextState;
                break;
            case lexaction.EndOfToken:
                //Emit a token
                Token tok = new Token(state, text.Substring(tokstart, pos - tokstart));
                tokstart = pos;
                state = t.NextState;
                yield return tok;
                break;
            default:
                state = t.NextState;
                break;
            }
        }
    }    
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
