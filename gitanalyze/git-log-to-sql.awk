BEGIN {
  print("n,email,company,date,ts,commit,subj," \
        "c_api,c_bpf,c_cilium,c_cli,c_common," \
        "c_contrib,c_daemon,c_envoy,c_hubble,c_operator," \
        " c_pkg,c_tools,c_plugins,c_proxylib,c_test," \
        "c_ignore,c_dump")
}
function emit() {
  n++;
  print(n "," email "," company "," date "," ts "," commit "," subj "," \
        c_api "," c_bpf "," c_cilium "," c_cli "," c_common "," \
        c_contrib "," c_daemon "," c_envoy "," c_hubble "," c_operator "," \
        c_pkg "," c_tools "," c_plugins "," c_proxylib "," c_test "," \
        c_ignore "," c_dump)  

  c_api=0
  c_bpf=0
  c_cilium=0
  c_cli=0
  c_common=0
  c_contrib=0
  c_daemon=0
  c_envoy=0
  c_hubble=0
  c_operator=0
  c_pkg=0
  c_tools=0
  c_plugins=0
  c_proxylib=0
  c_test=0
  c_ignore=0
  c_dump=0
  
  delete stat;
}

/^----$/  { emit() }
END       { emit() }
/^email/  {$1 = ""; gsub(/^ */,"",$0); email=$0; company=$0; gsub(/.*@/,"", company)}
/^date/   {$1 = ""; gsub(/^ */,"",$0); date=$0;}
/^ts/     {$1 = ""; gsub(/^ */,"",$0); ts=$0;}
/^commit/ {$1 = ""; gsub(/^ */,"",$0); commit=$0;}
/^subj/   {$1 = ""; gsub(/^ */,"",$0); subj=$0;}
/^$/      {/*$1 = ""; print("ignore");*/}

/^[0-9]+\s+[0-9]+/ {
  $1="";
  $2="";

  gsub(/^ */,"",$0);
  #print("stat:" $0);
  
  switch ($0) {
    case /api\//: c_api=1; break;
    case /bpf\//: c_bpf=1; break;
    case /cilium\//: c_cilium=1; break;
    case /cli\//: c_cli=1; break;
    case /common\//: c_common=1; break;
    case /contrib\//: c_contrib=1; break;
    case /daemon\//: c_daemon=1; break;
    case /Documentation\//: c_ignore=1; break;
    case /envoy\//: c_envoy=1; break;
    case /examples\//:  c_ignore=1; break;
    case /^hubble.\*/: c_hubble=1; break;
    case /images\//: c_ignore=1; break;
    case /install\//: c_ignore=1; break;
    case /operator\//: c_operator=1; break;
    case /pkg\//: c_pkg=1; break;# may need to break this down
    case /tools\//:  c_tools=1; break;
    case /plugins\//: c_plugins=1; break;
    case /proxylib\//: c_proxylib=1; break;
    case /test\//: c_test=1; break;
    case /tests\//: c_test=1; break;
    case /vendor\//: c_dump=1; break;
    case /^./: c_dump=1; break;
    default: c_other=1; break;
  }
  
  
  #print("stat" $0);
}

