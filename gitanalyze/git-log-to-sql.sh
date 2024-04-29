#!/bin/bash

set -e

top=$(dirname $0)
echo $top

echo <<EOF > /dev/null
----
email maderdario@gmail.com
date Tue Apr 4 08:12:20 2023 +0200
commit 56dfe3a5b8ea6c0e2bfd955fb29361948edbf3e8
xxx Ignore .github folder in .helmignore tats Ignore .github folder in .helmignore

1       0       install/kubernetes/cilium/.helmignore
----
email aspsk@isovalent.com
date Mon Apr 3 15:15:03 2023 +0000
commit 7ff2a65056f6ba03d459e238ad4d8a7761cb48b2
xxx bpf: simplify adding/removing types to alignchecker tats bpf: simplify adding/removing types to alignchecker

66      55      bpf/bpf_alignchecker.c
----
email timo@isovalent.com
date Mon Apr 3 15:35:41 2023 +0200
commit 60dfd1a7e5b1a548fd1c2f5dee9d140c07200b5f
xxx bpf: remove verifier-test.sh and check-complexity.sh tats bpf: remove verifier-test.sh and check-complexity.sh

0       2       CODEOWNERS
1       3       Documentation/contributing/development/dev_setup.rst
0       3       bpf/lib/common.h
0       66      test/bpf/check-complexity.sh
0       248     test/bpf/verifier-test.sh
EOF

# Get all potential file names for categorizing
# git log --numstat --format="" | awk '{$1="";$2="";print($0)}' > /tmp/out

 git log \
  --numstat \
  --format='----%nemail %aE%ndate %aI%nts %at%ncommit %H%nsubj %f' \
  | awk -f "${top}/git-log-to-sql.awk"
