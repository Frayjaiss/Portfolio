import pygame

#a dictionary that holds every square on our board. data looks like:
#a : [(Square1,Square1Color),(Square2,Square2Color)...]
#b : [(Square9,Square9Color),(Square10,Square10Color)...]
#.
#.
#.
boardPositions = {}

#draw the chess board
def build_board():
    boardPositions.clear() 
    #(x,y) offset for board
    boardOffset = (250,0)
    xMax: int = 8
    yMax: int = 8
    cellSize: int = 90
    fileNames = ["a","b","c","d","e","f","g","h"]
    colors = ["grey","black"]
    for xPos in range(0,xMax):
        currentFile = []
        for yPos in range(0,yMax):
            square = pygame.Rect((xPos*cellSize)+boardOffset[0],(yPos*cellSize)+boardOffset[1],cellSize,cellSize)
            tileColor = colors[(xPos+yPos)%2]
            currentFile.append((square,tileColor))
        #reverse the list so it matches with expected chess notation (a8 is the top left corner)
        currentFile.reverse()
        boardPositions[fileNames[xPos]] = currentFile


# pygame setup
def main():
    pygame.init()
    screen = pygame.display.set_mode((1280, 720))
    clock = pygame.time.Clock()
    running = True
    build_board()
    while running:
        # poll for events
        # pygame.QUIT event means the user clicked X to close your window
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        # fill the screen with a color to wipe away anything from last frame
        screen.fill("black")

        # RENDER GAME
        
        for key, value in boardPositions.items():
            for rectInfo in value:
                screen.fill(rectInfo[1],rectInfo[0])


        # flip() the display to put your work on screen
        pygame.display.flip()

        clock.tick(60)  # limits FPS to 60

    pygame.quit()

if __name__ == "__main__":
    main()
