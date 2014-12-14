#!/usr/bin/env python
# encoding: utf-8

preamble="""\\usetikzlibrary{{backgrounds,fit}}
\\begin{{tikzpicture}}
   [vertex/.style={{circle,draw=blue!50,fill=blue!20,thick}},
    selectedvertex/.style={{circle,draw=red!50,fill=red!20,thick}},
    edge/.style={{thick}},
    newedge/.style={{dashed,thick}}]"""

def component1(prefix, offset_x, offset_y, selectedvertexNo):
    def select_vertex(n):
        if n == selectedvertexNo:
            return "selectedvertex"
        else:
            return "vertex"

    def vertex(n, x, y):
        return "\\node[" + ("selectedvertex" if selectedvertexNo == n else "vertex") + "] (" + prefix + str(n) + ") at (" + str(x) + "," + str(y) + ") {" + str(n) + "}\n";
    
    ret = ""
    ret += vertex(1, offset_x + 2, offset_y + 2)
    ret += vertex(2, offset_x + 2, offset_y + 0)
    ret += vertex(3, offset_x + 4, offset_y + 1)
    ret += vertex(4, offset_x + 6, offset_y + 0)
    ret += vertex(5, offset_x + 6, offset_y + 2)
    ret += vertex(6, offset_x + 8, offset_y + 1)
    ret += vertex(7, offset_x + 0, offset_y + 1)
    #\draw[   edge] (1) to [out=270,in= 90] (2);
    #\draw[   edge] (1) to [out=  0,in=100] (3);
    #\draw[   edge] (1) to [out=180,in= 80] (7);
    #\draw[   edge] (2) to [out=180,in=280] (7);
    #\draw[   edge] (2) to [out=  0,in=260] (3);
    #\draw[   edge] (3) to [out=280,in=180] (4);
    #\draw[   edge] (3) to [out= 80,in=180] (5);
    #\draw[   edge] (4) to [out= 90,in=270] (5);
    #\draw[   edge] (4) to [out=  0,in=260] (6);
    #\draw[   edge] (5) to [out=  0,in=100] (6);
    return ret

    #return """
        #\\node[{vertexstyle1}] ({prefix}1) at ({x1},{y1}) {1};
        #\\node[{vertexstyle2}] ({prefix}2) at ({x2},{y2}) {2};
        #\\node[{vertexstyle3}] ({prefix}3) at ({x3},{y3}) {3};
        #\\node[{vertexstyle4}] ({prefix}4) at ({x4},{y4}) {4};
        #\\node[{vertexstyle5}] ({prefix}5) at ({x5},{y5}) {5};
        #\\node[{vertexstyle6}] ({prefix}6) at ({x6},{y6}) {6};
        #\\node[{vertexstyle7}] ({prefix}7) at ({x7},{y7}) {7};""".format(
        ##\\draw[   edge] ({prefix}1) to [out=270,in= 90] ({prefix}2);
        ##\\draw[   edge] ({prefix}1) to [out=  0,in=100] ({prefix}3);
        ##\\draw[   edge] ({prefix}1) to [out=180,in= 80] ({prefix}7);
        ##\\draw[   edge] ({prefix}2) to [out=180,in=280] ({prefix}7);
        ##\\draw[   edge] ({prefix}2) to [out=  0,in=260] ({prefix}3);
        ##\\draw[   edge] ({prefix}3) to [out=280,in=180] ({prefix}4);
        ##\\draw[   edge] ({prefix}3) to [out= 80,in=180] ({prefix}5);
        ##\\draw[   edge] ({prefix}4) to [out= 90,in=270] ({prefix}5);
        ##\\draw[   edge] ({prefix}4) to [out=  0,in=260] ({prefix}6);
        ##\\draw[   edge] ({prefix}5) to [out=  0,in=100] ({prefix}6);
        ##\\end{{tikzpicture}}""".format(
                #prefix=prefix,
                #vertexstyle1=select_vertex(1),
                #vertexstyle2=select_vertex(2),
                #vertexstyle3=select_vertex(3),
                #vertexstyle4=select_vertex(4),
                #vertexstyle5=select_vertex(5),
                #vertexstyle6=select_vertex(6),
                #vertexstyle7=select_vertex(7),
                #x1=offset_x+2,
                #x2=offset_x+2,
                #x3=offset_x+4,
                #x4=offset_x+6,
                #x5=offset_x+6,
                #x6=offset_x+8,
                #x7=offset_x+0,
                #y1=offset_y+2,
                #y2=offset_y+0,
                #y3=offset_y+1,
                #y4=offset_y+0,
                #y5=offset_y+2,
                #y6=offset_y+1,
                #y7=offset_y+1)

print(preamble)
print(component1(prefix="a", offset_x=0, offset_y=0, selectedvertexNo=3))
