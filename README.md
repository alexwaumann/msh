# msh (in development)

Mav shell (msh) is a basic shell program that I wrote for my Operating Systems class. It supports
up to 10 command line parameters in addition to the command.

Some Features:
  * ctrl-z suspends a process
  * ctrl-c stops the running process (won't kill msh)
  * `bg` will background a suspended process

Any command located in the following directories is supported:
  * .
  * /usr/local/bin
  * /usr/bin
  * /bin

Other commands that it supports:
  * cd
  * showpids
  * history (!n runs nth command)

It is implemented using fork(), wait(), and exec() functions.

To build the program on Mac or Linux:

```bash
$ git clone https://github.com/awaumann/msh.git
$ cd msh
$ make
$ ./msh
```