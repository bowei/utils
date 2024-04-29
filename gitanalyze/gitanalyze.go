package main

import (
	"bufio"
	"flag"
	"fmt"
	"os/exec"
	"sort"
	"strings"
)

var (
	ignore []string
)

// Common commands:
// --after="2022-01-01"

func main() {
	flag.Func(
		"ignore",
		"Ignore the directory. Can be used multiple times",
		func(s string) error { ignore = append(ignore, s); return nil })

	flag.Parse()
	gitPaths(flag.Args()...)

}

func gitPaths(args ...string) {
	args = append([]string{"log", "--numstat", `--format=`}, args...)
	fmt.Printf("# %+v\n", args)
	scanner, err := gitOutput(args...)
	if err != nil {
		panic(err)
	}
	files := map[string]bool{}
	for scanner.Scan() {
		fields := strings.Split(scanner.Text(), "\t")
		filename := fields[2]
		// EDIT: added\tdeleted\tfilename
		// MOVE: added\tdeleted\tprefix{src => dest}suffix
		if strings.Contains(filename, " => ") {
			// Ignore MOVE for now.
			continue
		}
		files[filename] = true
	}
	if err := scanner.Err(); err != nil {
		panic(err)
	}

	// Find directories from file names.
	//
	// Note: this will ignore the root directory, which we assume to be
	// not very interesting for analysis.
	dirs := map[string]int{}

fileLoop:
	for fn := range files {
		if !strings.Contains(fn, "/") {
			continue
		}
		// Chop off the filename at the end.
		pieces := strings.Split(fn, "/")
		pieces = pieces[:len(pieces)-1]

		// Ignore.
		dirName := strings.Join(pieces, "/")
		for _, ign := range ignore {
			if strings.HasPrefix(dirName, ign) {
				continue fileLoop
			}
		}

		// Count the commit for the path and all its parents.
		for i := 0; i < len(pieces)-1; i++ {
			dirName := strings.Join(pieces[:i], "/")
			dirs[dirName]++
		}
	}

	var dirNames []string
	for dn := range dirs {
		dirNames = append(dirNames, dn)
	}
	sort.Strings(dirNames)

	for _, dirName := range dirNames {
		fmt.Printf("%s: %d\n", dirName, dirs[dirName])
	}
}

func gitLog() {
	cmd := exec.Command("git", "-h")
	out, err := cmd.StdoutPipe()
	if err != nil {
		panic(err)
	}
	if err := cmd.Start(); err != nil {
		panic(err)
	}
	scanner := bufio.NewScanner(out)
	for scanner.Scan() {
		fmt.Println("XXX", scanner.Text())
	}
	if err := scanner.Err(); err != nil {
		panic(err)
	}
}

func gitOutput(args ...string) (*bufio.Scanner, error) {
	cmd := exec.Command("git", args...)
	out, err := cmd.StdoutPipe()
	if err != nil {
		return nil, err
	}
	if err := cmd.Start(); err != nil {
		return nil, err
	}
	return bufio.NewScanner(out), nil
}
