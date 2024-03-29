/****************************************************************************
 * A branch and bound optimization algorithm solving the 
 * "Take it Easy" board game
 * 
 * (c) Christian Wolf
 * Beginn: 26.12.2022
 ****************************************************************************/

#include <iostream>
#include <signal.h>

# define NO_SLOTS      19
# define MAX_NEIGHBORS  2

using namespace std;

void sig_handler(int);

class Jeton {
    public:
        Jeton () {
            n=0;                // n=0 indicates the empty slot
        }
        Jeton (unsigned char _n, unsigned char _w, unsigned char _e) {
            n=_n; w=_w; e=_e;        
        }
        unsigned char n,w,e;    // north, west, east numbers    

        friend ostream& operator <<(ostream &os, const Jeton& j);
};

ostream& operator <<(ostream &os, const Jeton& j) {
    if (j.n==0)
        os << "-";
    else
        os << "(" << (int) j.n << "," << (int) j.w << "," << (int) j.e << ")";
    return os;
}

Jeton board [NO_SLOTS];

// Eastern Neighbors of a given slot, in linear positions
int NE[NO_SLOTS][MAX_NEIGHBORS] =
{
  {-1,  1},     // 0  
  {0,  2},     // 1  
  {1,  -1},     // 2  
  {-1, 4},     // 3  
  {3,  5},     // 4  
  {4,  6},     // 5  
  {5,  -1},     // 6  
  {-1,  8},     // 7  
  {7,  9},     // 8  
  {8,  10},     // 9  
  {9,  11},     // 10  
  {10,  -1},     // 11 
  {-1,  13},     // 12   
  {12,  14},     // 13 
  {13,  15},     // 14 
  {14,  -1},     // 15  
  {-1, 17},     // 16 
  {16,  18},     // 17 
  {17,  -1},     // 18    
};

// Eastern Neighbors of a given slot, in linear positions
int NW[NO_SLOTS][MAX_NEIGHBORS] =
{
  {-1,  3},     // 0  
  {-1, 4},     // 1  
  {-1, 5},     // 2  
  {0,  7},     // 3  
  {1,  8},     // 4  
  {2,  9},     // 5  
  {-1, 10},     // 6  
  {3,  -1},     // 7  
  {4,  12},     // 8  
  {5,  13},     // 9  
  {6,  14},     // 10  
  {-1, 15},     // 11 
  {8,  -1},     // 12   
  {9,  16},     // 13 
  {10,  17},     // 14 
  {11, 18},     // 15  
  {13,  -1},     // 16 
  {14,  -1},     // 17 
  {15,  -1},     // 18    
};

// Northern Neighbors of a given slot, in linear positions
int NN[NO_SLOTS][MAX_NEIGHBORS] =
{
  {-1, 4},     // 0  
  {-1, 5},     // 1  
  {-1, 6},     // 2  
  {-1, 8},     // 3  
  {0,  9},     // 4  
  {1,  10},     // 5  
  {2,  11},     // 6  
  {-1,  12},     // 7  
  {3,  13},     // 8  
  {4,  14},     // 9  
  {5,  15},     // 10  
  {6, -1},     // 11 
  {7,  16},     // 12   
  {8,  17},     // 13 
  {9,  18},     // 14 
  {10,  -1},     // 15  
  {12,  -1},     // 16 
  {13,  -1},     // 17 
  {14,  -1},     // 18    
};

Jeton JetonStack[] = {
    Jeton(1,2,3),
    Jeton(5,7,3),
    Jeton(9,2,8),
    Jeton(9,7,8),
    Jeton(1,2,4),    
    Jeton(9,6,8),
    Jeton(5,2,3),
    Jeton(9,6,3),
    Jeton(5,7,8),
    Jeton(1,2,8),
    Jeton(1,6,3),
    Jeton(5,2,4),
    Jeton(9,2,4),
    Jeton(5,2,8),
    Jeton(9,2,3),
    Jeton(1,6,4),
    Jeton(5,6,4),
    Jeton(9,6,4),
    Jeton(1,7,8),
    Jeton(5,7,4),
    Jeton(1,7,3),
    Jeton(9,7,4),
    Jeton(5,6,3),
    Jeton(1,7,4),
    Jeton(5,6,8),
    Jeton(9,7,3),    
    Jeton(1,6,8)
};
int nbJetons = 27;

// For progress reporting and visualization
int maxDepth=-1;
Jeton *GSeen;
int GNbSeen;
int *counters;
long int totcnt=0;

// /****************************************************************************
//  Data structures for heuristics
//  ****************************************************************************/

#ifdef HEUR_HIST

class Hist {
    public:
        Hist () {
            for (int i=0; i<9; ++i)
                h[i]=0;
        }    
        void add (int n, int v) {
            h[n] += v;
        }
        void inc (int n) {
            h[n] += 1;
        }
        void dec (int n) {
            h[n] -= 1;
        }
        int h[9];
};

Hist HSeen, HNotSeen;

void updateHist (int p, Jeton *newJ, int value) {

    for (int i=0; i<2; ++i) {
        int n=NE[p][i];
        if (n!=-1) {
            // The neighbor is empty => we require a new number
            if (board[n].n==0)
                HSeen.add(newJ->e, value);                      
            // The neighbor is not empty => we remove a requirement
            else
                HSeen.add(newJ->e, 1*value);
        }
    }

    for (int i=0; i<2; ++i) {
        int n=NW[p][i];
        if (n!=-1) {
            // The neighbor is empty => we require a new number
            if (board[n].n==0)
                HSeen.add(newJ->w, value);                       
            // The neighbor is not empty => we remove a requirement
            else
                HSeen.add(newJ->w, -1*value);
        }
    }

    for (int i=0; i<2; ++i) {
        int n=NN[p][i];
        if (n!=-1) {
            // The neighbor is empty => we require a new number
            if (board[n].n==0)
                HSeen.add(newJ->n, value);                       
            // The neighbor is not empty => we remove a requirement
            else
                HSeen.add(newJ->n, -1*value);
        }
    }
}
#endif

/****************************************************************************
 Print a found solution
 ****************************************************************************/

void printSolution (Jeton *seen, int nbSeen) {    
    for (int i=0; i<3; ++i) 
        cout << i << ":" << board[i] << " ";
    cout << "\n";
    for (int i=3; i<7; ++i) 
        cout << i << ":" << board[i] << " ";
    cout << "\n";
    for (int i=7; i<12; ++i) 
        cout << i << ":" << board[i] << " ";
    cout << "\n";
    for (int i=12; i<16; ++i) 
        cout << i << ":" << board[i] << " ";
    cout << "\n";
    for (int i=16; i<19; ++i) 
        cout << i << ":" << board[i] << " ";
    cout << "\n";
}

/****************************************************************************
 Check whether a new piece n fits in at position p
 ****************************************************************************/

bool check (Jeton *seen, int nbSeen, int newp, Jeton *newJ) {

    // Go over the eastern neighbors
    for (int i=0; i<MAX_NEIGHBORS; ++i) {
        int p=NE[newp][i];

        // Empty slot, search the next neighbor in the same direction
        while ((p!=-1) && (board[p].n==0))
            p=NE[p][i];                
           
        // We found a valid non empty neighboring slot 
        if (p!=-1) {          
            if (board[p].e != newJ->e)
                return false;            
        }
    }

    // Go over the western neighbors
    for (int i=0; i<MAX_NEIGHBORS; ++i) {
        int p=NW[newp][i];

        // Empty slot, search the next neighbor in the same direction
        while ((p!=-1) && (board[p].n==0))
            p=NW[p][i];  
        
        if (p!=-1) {
            if ((board[p].n!=0) && (board[p].w != newJ->w))
                return false;
        }
    }

    // Go over the nothern neighbors
    for (int i=0; i<MAX_NEIGHBORS; ++i) {
        int p=NN[newp][i];

        // Empty slot, search the next neighbor in the same direction
        while ((p!=-1) && (board[p].n==0))
            p=NN[p][i];  

        if (p!=-1) {        
            if ((board[p].n!=0) && (board[p].n != newJ->n))
                return false;
        }
    }

    return true;
}

/****************************************************************************
 The main recursive function
 ****************************************************************************/

void F (Jeton *seen, int nbSeen, Jeton *notSeen, int nbNotSeen, int depth)
{
    GSeen = seen;
    GNbSeen = nbSeen;    
    ++totcnt;

#ifdef HEUR_HIST
    // Check heuristic: we need numbers which do not exist
    for (int i=0; i<9; ++i) 
        if (HSeen.h[i] > HNotSeen.h[i])
            return;            
#endif

    if (depth>maxDepth) {
        maxDepth = depth;
        cout << "New Max depth: " << maxDepth << endl;
    }
    
    if (nbSeen >= 19)
	{
        cout << "****** SOLUTION\n"; 
		printSolution(seen,nbSeen);
    }
    else {

        // Go through the unseen pieces
        for (int i=0; i<nbNotSeen; i++)
        {           
            counters[depth] = i;            

            // Go through the places and search for an empty one
            for (int p=0; p<NO_SLOTS; ++p) {
                if (board[p].n==0) {

                    // Check whether the i-th not seen piece positioned at 
                    // position p is compatible with the current solution
                    if (check(seen, nbSeen, p, notSeen+i)) {

                        // it is, move it in                        
                        board[p] = notSeen[i];
                        seen[nbSeen] = notSeen[i];
                        notSeen[i] = notSeen[nbNotSeen-1];

#ifdef HEUR_HIST                        
                        updateHist (p, notSeen+i, 1);
                        HNotSeen.dec(notSeen[i].e);
                        HNotSeen.dec(notSeen[i].w);
                        HNotSeen.dec(notSeen[i].n);
#endif                        
                                                                         
                        F(seen,nbSeen+1,notSeen,nbNotSeen-1, depth+1);

#ifdef HEUR_HIST
                        updateHist (p, notSeen+i, -1);
                        HNotSeen.inc(notSeen[i].e);
                        HNotSeen.inc(notSeen[i].w);
                        HNotSeen.inc(notSeen[i].n);
#endif
                        
                        notSeen[nbNotSeen-1] = notSeen[i];
                        notSeen[i] = seen[nbSeen];
                        board[p].n = 0;
                    }
                }
            }                        
        }
    }    
}

/****************************************************************************
 SIGINT signal: display status information
 ****************************************************************************/

void sig_handler(int sig) {
    switch (sig) {
    case SIGINT:
        cout << ("Caught signal: iterations for each depth:\n");
        for (int i=0; i<NO_SLOTS; ++i)
            cout << i << ":" << counters[i] << " ";
        cout << "\nCurrent partial solution:\n";
        printSolution(GSeen, GNbSeen);
        cout << "Total number of calls: " << totcnt << endl;
        break;        
    default:
        cout << "Unexpected signal!\n";
        abort();
    }
}

/****************************************************************************
 MAIN
 ****************************************************************************/

int main () {
    cout << "************* Take it easy.\n";

    signal(SIGINT, sig_handler);

    Jeton *Seen = new Jeton [nbJetons];
    counters = new int [nbJetons];

#ifdef HEUR_HIST
    cout << "Using histogram heuristics.\n";
    for (int i=0; i<nbJetons; ++i) {
        HNotSeen.inc (JetonStack[i].e);
        HNotSeen.inc (JetonStack[i].w);
        HNotSeen.inc (JetonStack[i].n);
    }
#endif   
    
    F (Seen, 0, JetonStack, nbJetons, 0);

    delete [] Seen;
    delete [] counters;
    cout << "Done.\n";
}