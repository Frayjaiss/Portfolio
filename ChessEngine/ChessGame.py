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

def game_startup(gameScreen,fenString):
    #build the current logical state of our board
    boardMap = BoardState.build_board_from_fen(fenString)
    #render the board
    RenderBoard.render_board(gameScreen,boardMap)
    return boardMap

# pygame setup
def main():
    running = True
    #get the screen for the game
    gameScreen = RenderBoard.initial_render()
    #build the starter board
    fenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
    boardMap = game_startup(gameScreen,fenString)
    while running:
        event = pygame.event.wait()
        if event.type == pygame.QUIT:
            running = False
            break

        #swapped to event driven loop. when pieces are interactable, event checks will go here.

        pygame.display.flip()



    pygame.quit()

if __name__ == "__main__":
    main()
