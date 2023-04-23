fun factorial(n : long) : long {
if n {
ret factorial(n - 1) * n;
} else {
ret 1;
}
}
fun main(): int {
let n : int = 6;
let res : long = factorial(n);
dbg res;
}