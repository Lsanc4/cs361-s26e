# Homework 2: Discovering increasingly low-level ways to greet the planet

In this assignment, we dig deeper into the rich world of, you guessed it, `Hello World!`. 
Starting with a familiar `printf` implementation, we use tools to discover how `printf` works.
Then we build a series of increasingly low-level implementations of this ever-popular piece of software,
learning a few things along the way. 

## Lab step 1: Build and run the glorious program

If you haven't already, clone this repo to your Linux environment, and change to the `hw2` folder. 
`make printf` builds the program for you. 

Have a look at the `Makefile`. It has no target for printf. Figure out why this works anyway.

*Demonstrate:* build and run the program, and explain to the TA why make knows how to build printf.

## Lab step 2: How does printf actually do it?

As we've learned in class, processes must use system calls to communicate with the outside world.
This includes putting characters on a screen! So which system call does printf use to do this? Let's find out.

## Lab step 3: Do it with `libc` `write()` instead

## Lab step 4: Do it with assembly instead

## Lab step 5: Do it with inline assembly instead

## Remaining step 6: Stop them from interrupting your work

## Remaining step 7: Interrupt their work

## Advanced remaining step 8: Handle SIGINT in Rust. 

Now write a Rust program instead. Start with `cargo new` to create a new project. Then 
add a signal handler. 

For this, we will need to add a library dependency, a `crate` in Rust speak to support signal handling.
There are several alternatives, including `libc`, `signal-hook` and `tokio::signal`, but we'll use `ctrlc` this time around: it's nice, and all we need. 

To add ctrlc as a dependency to your project, use `cargo add ctrlc`. Have a look at `Cargo.toml` to see what it did. There is documentation for this crate at https://crates.io/crates/ctrlc. Also, if you are unfamiliar with Rust, this website https://rust-lang.org/learn/ has excellent tutorials. 

To start, move the `println` line into a new function, called `myhandler`. Try calling it from `main` to make sure you got it right. Run the program with `cargo run`. 

Once that works, use `ctrlc::set_handler(myhandler);` to start listening for Ctrl-C, similar to how `sigaction` works in C. Naturally, `ctrlc` uses the same system calls that `sigaction` uses. 

You'll find that the program finishes immediately. To fix this, add a sleep interval with `std::thread::sleep`. https://doc.rust-lang.org/std/thread/fn.sleep.html

## Advanced remaining step 9: Count signals in Rust

Instead of the `myhandler` function, try passing an anonymous `move` closure that does the same thing. 
Once you get that to work, declare a mutable `count` variable in main, then have the closure increment the
count every time it is executed. To print the number, just add `{count}` to the `Hello World` string. 






