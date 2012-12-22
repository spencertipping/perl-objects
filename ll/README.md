# Low-level objects

These encapsulate various parts of the self-modifying logic and are suitable for building self-modifying
scripts that contain no shell. These scripts do not have friendly interfaces and generally must be edited
by using the 'perl' command.

If you want a script that works, I recommend using [object](../object).

## Functionality

These scripts are a work in progress.

- `tiny`: the smallest self-modifying Perl file I know how to write. Must be edited by hand.
- `persistent`: the smallest error-detecting Perl object that shares any architecture with `../object`.
  This object cannot be inherited from because it doesn't define the `cat` function.
- `small`: a version of `../object` with a number of interactive features removed.
