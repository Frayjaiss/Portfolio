import pygame
import math

#a dictionary that holds every square on our board. data looks like:
#0 : [(Square1,Square1Color),(Square2,Square2Color)...]
#1 : [(Square9,Square9Color),(Square10,Square10Color)...]
#.
#.
#.
boardPositions = {}

pieceAssets = []
fenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"

#draw the chess board
def build_board():
    boardPositions.clear() 
    #(x,y) offset for board
    boardOffset = (167,0)
    xMax: int = 8
    yMax: int = 8
    cellSize: int = 60
    colors = ["DarkSeaGreen","DarkGreen"]
    for yPos in range(0,xMax):
        currentFile = []
        for xPos in range(0,yMax):
            square = pygame.Rect((xPos*cellSize)+boardOffset[0],(yPos*cellSize)+boardOffset[1],cellSize,cellSize)
            tileColor = colors[(xPos+yPos)%2]
            currentFile.append((square,tileColor))
        boardPositions[yPos+1] = currentFile


def load_assets():
    allAssets = ["Assets\WhitePawn.png","Assets\BlackPawn.png"]
    for assetPath in allAssets:
        asset = pygame.image.load(assetPath).convert_alpha()
        pieceAssets.append(asset)


def get_cell_rect(row,col):
    pieceCell = boardPositions[int(row)][int(col)][0]
    return pieceCell

#reading the FEN string to create the board.
def place_pieces(screen):
    rowNum = 1
    colNum = 0
    for i in range(len(fenString)):
        currentChar = fenString[i]
        if currentChar == "/":
            rowNum += 1
            colNum = 0
            continue
        #if we read a number, these are empty cells and we can skip over them.
        if currentChar.isnumeric():
            #-1 because we add 1 at every iteration
            colNum += int(currentChar)
            continue
        elif currentChar == 'P':
            pieceCell = get_cell_rect(rowNum,colNum)
            screen.blit(pieceAssets[0],pieceCell)
        elif currentChar == 'p':
            pieceCell = get_cell_rect(rowNum,colNum)
            screen.blit(pieceAssets[1],pieceCell)
        colNum += 1



# pygame setup
def main():
    pygame.init()
    screen = pygame.display.set_mode((853, 480))
    clock = pygame.time.Clock()
    running = True
    build_board()
    load_assets()
    while running:
        # poll for events
        # pygame.QUIT event means the user clicked X to close your window
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        # fill the screen with a color to wipe away anything from last frame
        screen.fill("black")

        # RENDER GAME
        #start with the board
        for key, value in boardPositions.items():
            for rectInfo in value:
                screen.fill(rectInfo[1],rectInfo[0])

        #then the pieces
        place_pieces(screen)
        # flip() the display to put your work on screen
        pygame.display.flip()

        clock.tick(20)  # limits FPS to 20

    pygame.quit()

if __name__ == "__main__":
    main()
