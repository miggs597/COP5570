# Miguel A. Perez Ojito
## COP5570

On linprog please run:

```tsch
scl enable devtoolset-11 bash
```

as I am compiling with -std=c++20




Project 3
In the client a single pthread is used so that it can
write to the server and receive messages at the same time

In the server each client gets its own thread, that way we are
stuck attending to one client at a time.
