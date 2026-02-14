#this is the main game loop of the chess game. It calls upon the methods of the other files, and sends the necessary information to them.
#the main game flow is as follows: receive_fen_string -> BoardState.build_board -> RenderBoard.render_board -> await user input.

#User input is 2 step: selection and movement. Upon selection, we call BoardState.identify_piece to find the type of piece 
#selected by the user. We then call engine.calc_moves to find the legal moves for that piece. Afterwards, another selection input can be 
#given, or the user can make a move with that piece. Making a move means pressing on a cell that contains a different colored piece.
#We can determine that using BoardState.identify_piece again. if the cell is empty, or the piece is opposite colored, we are making a 
#move action. We analyze the desired move against the engine.calc_moves to see if it is contained within the returned set. If it is,
#the move is made and turn is passed to the engine. Otherwise, the move is rejected.

import pygame
import RenderBoard
import BoardState
import ChessPieces

selectedCell = None
validMoves = []



def game_startup(gameScreen,board):
    #build the current logical state of our board
    boardMap = board.build_board_from_fen()
    #render the board
    RenderBoard.render_board(gameScreen,boardMap)
    return boardMap


#two possibilities for LMB: either we are trying to select a piece or move a piece.
#the former is true if we selected a cell outside of valid moves and we chose a cell with a piece on it
#the later is true if we selected a cell inside valid moves
def left_mouse_button(pos:tuple,board:BoardState.ChessBoard):
    closestCell = board.find_closest_cell(pos,True)
    if closestCell == None:
        pass
    else:
        global selectedCell
        global validMoves
        activeColor = board.parsedFen.get("ActiveColor")
        #select a new piece
        if (not closestCell in validMoves) and (closestCell[2] != None) and (activeColor == closestCell[2].color):
            selectedCell = closestCell
            validMoves = closestCell[2].piece_clicked()
        #move a piece
        elif (closestCell in validMoves) and (selectedCell != None):
            selectedCell[2].move_piece(closestCell)
            selectedCell = None
            validMoves = []
            #swap avctive color after a move
            if activeColor == "White":
                board.parsedFen["ActiveColor"] = "Black"
            else:
                board.parsedFen["ActiveColor"] = "White"
        #if we reach here, we clicked an empty cell. Reset our trakcing variables.
        else:
            selectedCell = None
            validMoves = []      
    


# pygame setup
def main():
    running = True
    #get the screen for the game
    gameScreen = RenderBoard.render_setup()
    #build the starter board
    chessBoard = BoardState.ChessBoard((167,0),60,8,8,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w")
    boardMap = game_startup(gameScreen,chessBoard)
    while running:
        event = pygame.event.wait()
        if event.type == pygame.QUIT:
            running = False
            break
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            #this function sets the global variables selectedCell and validMoves
            left_mouse_button(event.pos,chessBoard)
                
        #Render any board changes
        RenderBoard.render_board(gameScreen,boardMap)
        #draw any new move indicators       
        RenderBoard.show_move_indicators(gameScreen,validMoves)

        pygame.display.flip()

    pygame.quit()

if __name__ == "__main__":
    main()
