{nixpkgs, stdenv, dysnomia, disnix}:

let
  manifestTests = ./manifest;
  machine = import ./machine.nix { inherit dysnomia disnix; };
  wrapper = import ./snapshots/wrapper.nix { inherit stdenv dysnomia; };
in
with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };

  simpleTest {
    nodes = {
      client = machine;
      server = machine;
    };
    testScript = 
      ''
        startAll;
        
        #### Test disnix-ssh-client's snapshot operations
        
        # Initialise ssh stuff by creating a key pair for communication
        my $key=`${pkgs.openssh}/bin/ssh-keygen -t dsa -f key -N ""`;
    
        $server->mustSucceed("mkdir -m 700 /root/.ssh");
        $server->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");
        
        $client->mustSucceed("mkdir -m 700 /root/.ssh");
        $client->copyFileFromHost("key", "/root/.ssh/id_dsa");
        $client->mustSucceed("chmod 600 /root/.ssh/id_dsa");
        
        $server->waitForJob("sshd");
        $client->mustSucceed("sleep 10"); # !!! Delay hack
        
        # Activate the wrapper component
        $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --activate --type wrapper ${wrapper}");
        
        # Take a snapshot
        $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}");
        
        # Make a change and take another snapshot
        $server->mustSucceed("echo 1 > /var/db/wrapper/state");
        $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}");
        
        # Make another change and take yet another snapshot
        $server->mustSucceed("echo 2 > /var/db/wrapper/state");
        $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}");
        
        # Query all snapshots. We expect three of them.
        my $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --query-all-snapshots --container wrapper --component ${wrapper} | wc -l");
        
        if($result == 3) {
            print "We have 3 snapshots!\n";
        } else {
            die "Expecting 3 snapshots!";
        }
        
        # Query latest snapshot. The resulting snapshot text must contain 2.
        $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --query-latest-snapshot --container wrapper --component ${wrapper}");
        my $lastSnapshot = substr $result, 0, -1;
        my $lastResolvedSnapshot = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --resolve-snapshots $lastSnapshot");
        $result = $server->mustSucceed("cat ".(substr $lastResolvedSnapshot, 0, -1)."/state");
        
        if($result eq "2\n") {
            print "Result is 2\n";
        } else {
            die "Result should be 2!";
        }
        
        # Print missing snapshots. The former path should exist while the latter
        # should not, so it should return only one snapshot.
        
        $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --print-missing-snapshots --container wrapper --component ${wrapper} $lastSnapshot wrapper/wrapper/foo | wc -l");
        
        if($result == 1) {
            print "Result is 1\n";
        } else {
            die "Result should be 1!";
        }
        
        # Export snapshot. This operation should fetch the latest snapshot from
        # the server, containing the string 2.
        
        $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --export-snapshots ".(substr $lastResolvedSnapshot, 0, -1));
        my $tmpDir = $client->mustSucceed("echo ".(substr $result, 0, -1)."/\$(basename $lastSnapshot)");
        $result = $client->mustSucceed("cat ".(substr $tmpDir, 0, -1)."/state");
        
        if($result == 2) {
            print "Result is 2\n";
        } else {
            die "Result should be 2!";
        }
        
        # Make another change and take yet another snapshot
        $server->mustSucceed("echo 3 > /var/db/wrapper/state");
        $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --snapshot --type wrapper ${wrapper}");
        
        # Run the garbage collector. After running it only one snapshot should
        # be left containing the string 3
        $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --clean-snapshots --container wrapper --component ${wrapper}");
        $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --query-all-snapshots --container wrapper --component ${wrapper} | wc -l");
        
        if($result == 1) {
            print "We only 1 snapshot left!\n";
        } else {
            die "Expecting 1 remaining snapshot!";
        }
        
        $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --query-latest-snapshot --container wrapper --component ${wrapper}");
        $lastSnapshot = substr $result, 0, -1;
        $lastResolvedSnapshot = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --resolve-snapshots $lastSnapshot");
        $result = $server->mustSucceed("cat ".(substr $lastResolvedSnapshot, 0, -1)."/state");
        
        if($result eq "3\n") {
            print "Result is 3\n";
        } else {
            die "Result should be 3!";
        }
        
        # Import the snapshot that has been previously exported and check if
        # there are actually two snapshots present.
        $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --import-snapshots --localfile --container wrapper --component wrapper ".(substr $tmpDir, 0, -1));
        
        $result = $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --query-all-snapshots --container wrapper --component ${wrapper} | wc -l");
        
        if($result == 2) {
            print "Result is 2\n";
        } else {
            die "Result should be 2!";
        }
        
        # Make another change
        $server->mustSucceed("echo 4 > /var/db/wrapper/state");
        
        # Restore the last snapshot and check whether it has the previously
        # uploaded state (2)
        $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --restore --type wrapper ${wrapper}");
        $result = $server->mustSucceed("cat /var/db/wrapper/state");
        
        if($result == 2) {
            print "Result is 2\n";
        } else {
            die "Result should be 2!";
        }
        
        # Deactivate the component
        $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --deactivate --type wrapper ${wrapper}");
        
        # Delete the state and check if it is not present anymore.
        $client->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' disnix-ssh-client --target server --delete-state --type wrapper ${wrapper}");
        $server->mustSucceed("[ ! -e /var/db/wrapper ]");
      '';
  }
