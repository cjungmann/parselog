# Parselog Project

A [BASH-based Syslog scanner](https://github.com/cjungmann/iptables_firewall)
was too slow, so this C project is a learning project to see how
much better C will perform on this task.

## History of Project

Inspired by finding an unnoticed iptables error on my firewall, I created
a [BASH-based program](https://github.com/cjungmann/iptables_firewall)
for scanning /var/log/syslog for iptables entries.

Unfortunately, this program ended up being much too slow for my liking.
I thought I could write the same thing in C without too much trouble.

As I worked on the stuff that was easy in BASH but not so much in C,
I created some modules that I can use in other projects.  I organized
the code to allow choosing either stack- or heap-based memory.  I added
a timer and an option to run a configuration many times to quantify
performances between different memory allocation methods.

