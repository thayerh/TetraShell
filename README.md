---
title: TETRASHELL
section: 1
header: Assignment 5 - Tetrashell
footer: Tetrashell 1.0.0
date: May 2, 2023
---
# NAME
Tetrashell - an all-in-one command line interface tetris quicksave hacking tool. 

# SYNOPSIS
**Tetrashell** [OPTION]...[ADD'l ARUGMENTS IF NEEDED]...

# DESCRIPTION
**Tetrashell** is a multi-faceted tetris quicksave modifier. Some features include modify, rank, recover, and check. (See **NOTES** for more details) This is all run through a custom shell built into a command line interface. 

The CLI will look like this "(currentUser)@TShell[QuicksaveName][score/lines]>"

A user will be prompted with entering the path to their quicksave at the start of the program. After the input there are multiple commands a user can run.

All commands can be entered with the shortest amount of characters it takes to uniquely identitfy it.

**-check** 
: This command calls the **check** program with the current quicksave to verify if it will pass legitimacy checks.

**-rank** 
: Rank the current quicksave with a database of other saves." <"Rank or 'ra', etc."> <'Score' or 'Lines'> and <"number of lines to return."> Can just input **rank** and will default to 5 lines on either side of current file and sort by score.

**-modify**
Modifies the current save. Input <**Modify** or 'm', 'mo', etc.> <"Score or Lines"> <"Number to set value to">.

**-switch**
Switches the current save to the one you input. Input <**Switch**> <"Save path">.

**-recover**
Recovers a list of quicksaves found in a file provided by the user. Offers the option to switch to one of the recovered quicksaves.

**-help**
: type help to display a message on further instructions on how to use each feature in **tetrashell** Ex. 'help check'.

**-info**
: Displays the info of the given save. Includes scores, lines, and validity. No arguments taken. 

**-visualize**
Prints the visual description of the given save.

**-undo** 
: This can be called after a modify command that will revert the savefile to the last version before it was modified. 

**-train**
: This is a fun game unrelated to the other functions of **tetrashell**. This game gives you either a random integer, binary, or hexidecimal number representation and will test if you can convert it into one of the other two versions. Type 'exit' to quit at anytime. 

# EXAMPLES
*Normal Use:*

**userName@TShell[s.bin][0/2]>** modify score 100

"Quicksave s.bin modified."

**userName@TShell[s.bin][100/2]>** undo

**userName@TShell[s.bin][0/2]>** check

The quicksave s1.bin appears legitimate.

**userName@TShell[s.bin][0/2]>** exit

*Train:*

example@TShell[save...][420000/4000]> train

Welcome to train. type 'exit' to quit

Convert hex number: 0032 into binary -> <"userinput"> (138)

No, the correct answer is: 00110010

Convert hex number: 00A9 into binary -> 10101001

Correct!

Given binary: 00110011 convert to integer -> exit

Thanks for playing.

example@TShell[save...][420000/4000]> 



# AUTHORS
Kishan Patel and Thayer Hicks

# NOTES
Features implemented: modify, rank, recover, check, switch, undo, visualize, improved prompt, intro animation
, quick-rank, help, info, train, short-commands, pretty-rank, and pretty-recover.

Short commands allow for users to input  'ra', 'ran', etc. all for rank. This also applies for other commands where they can be shortened to the lowest amount. 
Ex. 'ra' for rank becuase 'r' would qualify for recover and rank. This was implemented through the inputCheck() function.


No further steps are needed to see any of the other extra features.

Furthermore, there has been an additional implementation to check the quicksave, 
and if valid, the filepath within the prompt will be green. Otherwise, it will be red. 
This allows for the user to quickly see if their save is vaild after modifying. No extra steps needed.
There is also an intro animation that slides the logo in from the side. Lastly, there was an addition of whether the save is vaild in
the basic save info.  

*COLOR IS NOT PRINTED ON UNSUPPORTED TERMINALS. 

# SEE ALSO 
Open source for all code found at https://github.com/thayerh/COMP211-A5 
