package gitql

import (
	"fmt"
	"strings"

	git "github.com/go-git/go-git/v5"
	"github.com/go-git/go-git/v5/plumbing"
	"github.com/go-git/go-git/v5/plumbing/object"
)

// Do the parse on the repository at filesystem `path`, starting with the commit
// `ref`. See https://github.com/src-d/go-git/blob/master/plumbing/reference.go
// for the reference string format.
func Do(path, ref string) error {
	repo, err := git.PlainOpen(path)
	if err != nil {
		return err
	}
	p := &parser{repo: repo, ref: ref}
	return p.run()
}

type parser struct {
	repo *git.Repository
	ref  string
}

func (p *parser) run() error {
	ref, err := p.repo.Reference(plumbing.ReferenceName(p.ref))
	if err != nil {
		return err
	}
	citr, err := p.repo.Log(&git.LogOptions{From: ref.Hash()})
	if err != nil {
		return err
	}
	var (
		strandSeq int
	)
	err = citr.ForEach(func(c *object.Commit) error {
		strandSeq++
		parent, err := c.Parent(0)
		if err != nil {
			return err
		}

		hash := c.Hash
		strand := "strand1"
		email := c.Author.Email
		when := c.Author.When.String()
		parentHash := parent.Hash

		fmt.Printf(`
INSERT INTO commits VALUES (
	'%s',
	'%s',
	'%d',
	'%s',
	'%s',
	'%s',
	'%s')`, hash, strand, strandSeq, email, "", when, parentHash)

		patch, err := c.Patch(parent)
		if err != nil {
			return err
		}

		for _, fs := range patch.Stats() {
			dirName := "/"
			parts := strings.Split(fs.Name, "/")
			if len(parts) > 1 {
				dirName = strings.Join(parts[:len(parts)-1], "/")
			}

			fmt.Printf(`
INSERT INTO commitFiles VALUES (
	'%s',
	'%s',
	'%s',
	%d,
	%d)`, hash, fs.Name, dirName, fs.Addition, fs.Deletion)
		}
		return nil
	})
	if err != nil {
		return err
	}

	return nil
}
