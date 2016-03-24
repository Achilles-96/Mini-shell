

shell: shell.c echo.c prompt.c cd.c whoami.c
	gcc shell.c echo.c prompt.c cd.c whoami.c -I Includes -o shell
