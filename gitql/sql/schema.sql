CREATE TABLE commits (
  hash TEXT PRIMARY KEY
  startCommit TEXT
  seq INTEGER
  email TEXT
  emailDomain TEXT
  when TEXT -- datetime
  parentHash TEXT
)

CREATE TABLE commitFiles (
    hash TEXT
    file TEXT
    dir TEXT
    add INTEGER
    remove INTEGER
)
