# Game-AI-with-Minimax


**Game Rules**

 ![image](https://github.com/boy860521/Game-AI-with-Minimax/blob/master/board.png)<br>
1. Each player has 9 pieces initially, arranged as in image (a).<br>
2. Black moves first.<br>
3. The valid moves (see image (b)) are:<br>
(1) Move one piece to an adjacent unoccupied square.<br>
(2) Have one piece hop over another piece of either player to an unoccupied square. Multiple hops by one piece can be taken within one move as long as all the hops are valid and form a sequence. However, no loop is allowed; you cannot hop to the same square twice within one move.<br>
4. When hopping over an opponent's piece, the opponent's piece is considered "captured" and removed from the board.
5. The overall objective is to move as many of a player's pieces as possible to the shaded squares in the opposite side (the target region).
6. A player's move is skipped if he/she makes an invalid move or has no piece remaining on the board. The other player will continue to play.
7. The game ends when Either player has all his/her pieces on the board in the target region, or A maximum of 200 moves per player has been played.
8. At the end, the score of a player is the number of pieces placed in the target region. The player with the higher score wins the game.

**Note**
> This program is writen in C++11 and using Visual Studio 2017, so one might need to specify it when compiling this.  
