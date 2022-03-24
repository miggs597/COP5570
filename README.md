# Miguel A. Perez Ojito
## COP5570

On linprog please run:

```tsch
scl enable devtoolset-9 bash
```

as I am compiling with -std=c++17




Project 2
I choose to have the builtin commands as internal commands.
Because of how small they were, usually just a one liner it 
didn't make sense to me to have a bespoke file for each builtin 
command. So it was just easier to have mytoolkit be a monolith,
designing it as a monolith allowed me to quikcly debug and deploy 
new functionality as I worked on the project. It also makes for a dead
simple compilation process, as I only have one file that needs to
be compiled. 