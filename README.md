# Branch and bound solution of the "Take it easy" board game

The [Take it Easy](https://boardgamegeek.com/boardgame/128/take-it-easy) board game requires to fill a board with chips of constant straight lines. This code calculates all optimal solutions with branch and bound.

**Caveat**: 
- the brute force solution probably requires a couple of thousands of years to finish on my current M1 Macbook.
- If you compile with -DHEUR_HIST (current configuration of the Makefile), it adds a number histogram heuristic which cuts branches when the numbers required by the current partial solution are not contained in the stack of the available chips. But even this heuristic cuts down calculations to a couple of years.
- There is a possibility of a new heuristic, but I found it too complex to implement, since too many configurations need to be manually coded: actually check what kind of chips are available and if they fit the pattern of open slots of the partial solution. But: ever open slot may have a variable amount of available or empty slots around it, requiring to consider lots of cases.

