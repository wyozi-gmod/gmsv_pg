workflow "build" {
  on = "push"
  resolves = "compile-linux"
}

action "compile-linux" {
  uses = "./docker-compile/linux"
}
