#/usr/bin/env perl
# $Id: sshserver.pl,v 1.21 2007-09-17 21:39:34 danf Exp $
# Starts sshd for use in the SCP, SFTP and SOCKS curl test harness tests.
# Also creates the ssh configuration files (this could be moved to a
# separate script).

# Options:
# -u user
# -v
# target_port

use strict;
use File::Spec;

my $verbose=0; # set to 1 for debugging

my $port = 8999;        # just our default, weird enough
my $listenaddr = "127.0.0.1"; # address on which to listen

my $path = `pwd`;
chomp $path;

my $exeext;
if ($^O eq 'MSWin32' || $^O eq 'cygwin' || $^O eq 'msys' || $^O eq 'dos' || $^O eq 'os2') {
    $exeext = '.exe';
}

# Where to look for sftp-server
my @sftppath=qw(/usr/lib/openssh /usr/libexec/openssh /usr/libexec /usr/local/libexec /opt/local/libexec /usr/lib/ssh /usr/libexec/ssh /usr/sbin /usr/lib /usr/lib/ssh/openssh /usr/lib64/ssh /usr/lib64/misc /usr/lib/misc);

my $username = $ENV{USER};

# Find a file somewhere in the given path
sub searchpath {
  my $fn = $_[0] . $exeext;
  shift;
  my @path = @_;
  foreach (@path) {
      my $file = File::Spec->catfile($_, $fn);
      if (-e $file) {
          return $file;
      }
  }
}

# Parse options
do {
    if($ARGV[0] eq "-v") {
        $verbose=1;
    }
    elsif($ARGV[0] eq "-u") {
        $username=$ARGV[1];
        shift @ARGV;
    }
    elsif($ARGV[0] eq "-l") {
        $listenaddr=$ARGV[1];
        shift @ARGV;
    }
    elsif($ARGV[0] =~ /^(\d+)$/) {
        $port = $1;
    }
} while(shift @ARGV);

my $conffile="curl_sshd_config";    # sshd configuration data
my $conffile_ssh="curl_ssh_config";    # ssh configuration data
my $knownhostsfile="curl_client_knownhosts";    # ssh knownhosts file

# Searching for sshd and sftp-server will be done first
# in the PATH and afterwards in other common locations.
my @spath;
push(@spath, File::Spec->path()); 
push(@spath, @sftppath); 

# sshd insists on being called with an absolute path.
my $sshd = searchpath("sshd", @spath);
if (!$sshd) {
    print "sshd$exeext not found\n";
    exit 1;
}
if ($verbose) {
    print STDERR "SSH server found at $sshd\n";
}

my $sftp = searchpath("sftp-server", @spath);
if (!$sftp) {
    print "Could not find sftp-server$exeext plugin\n";
    exit 1;
}
if ($verbose) {
    print STDERR "SFTP server plugin found at $sftp\n";
}

if ($username eq "root") {
    print "Will not run ssh daemon as root to mitigate security risks\n";
    exit 1;
}

# Find out sshd version.
my $tmpstr;
my $ssh_daemon;
my $ssh_ver_major;
my $ssh_ver_minor;
my $ssh_ver_patch;
chomp($tmpstr = qx($sshd -V 2>&1 | grep OpenSSH));
if ($tmpstr =~ /OpenSSH[_-](\d+)\.(\d+)(\.(\d+))*/) {
    ($ssh_ver_major, $ssh_ver_minor, $ssh_ver_patch) = ($1, $2, $4);
    $ssh_daemon = 'OpenSSH';
}
if ($verbose) {
    print STDERR "ssh_daemon: $ssh_daemon\n";
    print STDERR "ssh_ver_major: $ssh_ver_major\n";
    print STDERR "ssh_ver_minor: $ssh_ver_minor\n";
    print STDERR "ssh_ver_patch: $ssh_ver_patch\n";
}

# Verify minimum OpenSSH version.
if (($ssh_daemon !~ /OpenSSH/) || (10 * $ssh_ver_major + $ssh_ver_minor < 37)) {
    print "SCP, SFTP and SOCKS tests require OpenSSH 3.7 or later\n";
    exit 1;
}

# Initialize sshd configuration file for curl's tests.
open(CONF, ">$conffile") || die "Could not write $conffile";
print CONF "# This is a generated file!  Do not edit!\n";
print CONF "# OpenSSH sshd configuration file for curl testing\n";
close CONF;

# Support for some options might have not been built into sshd.  On some
# platforms specifying an unsupported option prevents sshd from starting.
# Check here for possible unsupported options, avoiding its use in sshd.
sub sshd_supports_opt($) {
    my ($option) = @_;
    my $err = grep /Unsupported .* $option/, qx($sshd -t -f $conffile -o $option=no 2>&1);
    return !$err;
}

my $supports_UsePAM = sshd_supports_opt('UsePAM');
my $supports_UseDNS = sshd_supports_opt('UseDNS');
my $supports_ChReAu = sshd_supports_opt('ChallengeResponseAuthentication');
if ($verbose) {
    print STDERR "sshd supports UsePAM: ";
    print STDERR $supports_UsePAM ? "yes\n" : "no\n";
    print STDERR "sshd supports UseDNS: ";
    print STDERR $supports_UseDNS ? "yes\n" : "no\n";
    print STDERR "sshd supports ChallengeResponseAuthentication: ";
    print STDERR $supports_ChReAu ? "yes\n" : "no\n";
}

if (! -e "curl_client_key.pub") {
    if ($verbose) {
        print STDERR "Generating host and client keys...\n";
    }
    # Make sure all files are gone so ssh-keygen doesn't complain
    unlink("curl_host_dsa_key", "curl_client_key","curl_host_dsa_key.pub", "curl_client_key.pub"); 
    system "ssh-keygen -q -t dsa -f curl_host_dsa_key -C 'curl test server' -N ''" and die "Could not generate host key";
    system "ssh-keygen -q -t dsa -f curl_client_key -C 'curl test client' -N ''" and die "Could not generate client key";
}

open(FILE, ">>$conffile") || die "Could not write $conffile";
print FILE <<EOFSSHD
AllowUsers $username
DenyUsers
DenyGroups
AuthorizedKeysFile $path/curl_client_key.pub
HostKey $path/curl_host_dsa_key
PidFile $path/.ssh.pid
Port $port
ListenAddress $listenaddr
Protocol 2
AllowTcpForwarding yes
GatewayPorts no
HostbasedAuthentication no
IgnoreRhosts yes
IgnoreUserKnownHosts yes
KeepAlive no
PasswordAuthentication no
PermitEmptyPasswords no
PermitUserEnvironment no
PermitRootLogin no
PrintLastLog no
PrintMotd no
StrictModes no
Subsystem sftp $sftp
UseLogin no
PrintLastLog no
X11Forwarding no
UsePrivilegeSeparation no
# Newer OpenSSH options
EOFSSHD
;
close FILE ||  die "Could not close $conffile";

sub set_sshd_option {
    my ($string) = @_;
    if (open(FILE, ">>$conffile")) {
        print FILE "$string\n";
        close FILE;
    }
}

if ($supports_UsePAM) {
    set_sshd_option('UsePAM no');
}
if ($supports_UseDNS) {
    set_sshd_option('UseDNS no');
}
if ($supports_ChReAu) {
    set_sshd_option('ChallengeResponseAuthentication no');
}


# Now, set up some configuration files for the ssh client
open(DSAKEYFILE, "<curl_host_dsa_key.pub") || die 'Could not read curl_host_dsa_key.pub';
my @dsahostkey = do { local $/ = ' '; <DSAKEYFILE> };
close DSAKEYFILE || die "Could not close DSAKEYFILE";

open(KNOWNHOSTS, ">$knownhostsfile") || die "Could not write $knownhostsfile";
print KNOWNHOSTS "[$listenaddr]:$port ssh-dss $dsahostkey[1]\n" || die 'Could not write to KNOWNHOSTS';
close KNOWNHOSTS || die "Could not close KNOWNHOSTS";

open(SSHFILE, ">$conffile_ssh") || die "Could not write $conffile_ssh";
print SSHFILE <<EOFSSH
IdentityFile $path/curl_client_key
UserKnownHostsFile $path/$knownhostsfile
StrictHostKeyChecking no
Protocol 2
BatchMode yes
CheckHostIP no
Compression no
ConnectTimeout 20
ForwardX11 no
HostbasedAuthentication yes
NoHostAuthenticationForLocalhost no
# Newer OpenSSH options
#SetupTimeOut 20
EOFSSH
;
close SSHFILE ||  die "Could not close $conffile_ssh";


if (system "$sshd -t -q -f $conffile") {
    # This is likely due to missing support for UsePam
    print "$sshd is too old and is not supported\n";
    unlink $conffile;
    exit 1;
}

# Start the server
my $rc = system "$sshd -e -D -f $conffile > log/sshd.log 2>&1";
$rc >>= 8;
if($rc && $verbose) {
    print STDERR "$sshd exited with $rc!\n";
}

unlink $conffile;

exit $rc;
