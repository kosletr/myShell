# myShell

## Description

This work was created for the scope of an assignment in a Operating Systems cource of the Aristotle University of Thessaloniki during the 2018-19 academic year. The main purpose of this project is to demonstrate the implementation of a Linux shell in C created by me, Konstantinos Letros.

<p align="center">
  <img src="https://github.com/kosletr/myShell/blob/master/shell.png">
</p>

## Getting Started - Run the code

To run the shell, just do the following

1) Unzip files
2) Open terminal and browse to the extracted directory
3) Run make
4) Run ./shell

## Functionalities


- Built-in functions
  - quit : exit shell
  - cd : browse directories
  
- System call commands
  - example: ls -l
  - example: echo
  
- Support for multiple commands on every line
  - example: ls -l ; pwd
  - example: ls -l && pwd
  
- Pipelines
  - example: ls | wc -l
  
- Redirections
  - example: echo first line > file.txt
  - example: echo second line >> file.txt
  - example: echo < file.txt
 
- Combinations of the above
  - example: echo first line > file.txt ; echo second line >> file.txt && cat file.txt | grep second
