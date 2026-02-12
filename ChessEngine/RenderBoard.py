#this file is responsible for reading our board state and then rendering what that board would look like graphically. It will
# render the board cells themselves at the start of the program, and will render pieces in the correct positions. It will also
#be responsible for showing valid moves when a piece is clicked on.

import pygame

#initiates the game and returns the screen to render to.
def initial_render():
    pygame.init()
    screen = pygame.display.set_mode((853, 480))
    return screen


#render the board constructed in build_board_rects to the given screen. Each entry in boardMap is an array as follows:
#[CellLocation,CellColor,PiecePresent]
def render_board(screen,boardMap):
    screen.fill("black")
    for key in boardMap:
        for rectInfo in boardMap[key]:
            screen.fill(rectInfo[1],rectInfo[0])
            if rectInfo[2] != None:
                screen.blit(rectInfo[2].asset,rectInfo[0])
