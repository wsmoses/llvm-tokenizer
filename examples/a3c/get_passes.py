import re 

def get_passes(fn='opt_passes.md'):
    passes = []
    with open(fn) as fp:
        for line in fp:
            tokens = line.split('|');
            #print (len(tokens))
            if tokens[2].strip()== "Y":
                pass_name = tokens[0]
                pass_name = pass_name.strip().replace('`', '')
                passes.append(pass_name)
                
    print(passes)
    print("Number of Passes: %d" % len(passes))
    return (passes, len(passes))

#get_passes()  
