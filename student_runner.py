import argparse
import os
import subprocess
parser = argparse.ArgumentParser(description='You pass this script three things: 1) A file with a list of NetIDs 2) A bash script to execute in the students folder 3) The location of the students folder (using {NETID} as a placeholder for each person\'s NetID')
parser.add_argument('netids', help='A file containing a NetID on each line')
parser.add_argument('script', help='The script to run')
parser.add_argument('folderLocation', help='The location of each students folder')

args = parser.parse_args()
assert('{NETID}' in args.folderLocation)
netid_file_object = open(args.netids, 'r')
#just opening this to make sure it exists
script_file_object = open(args.script, 'r')
netids = []
for line in netid_file_object:
    if len(line) > 0:
        netid = line.strip()
        student_location = args.folderLocation.replace('{NETID}', netid)
        assert(os.path.isdir(student_location))
        netids.append(netid)
absolute_script_path = os.path.abspath(args.script)
current_location = os.getcwd()
for netid in netids:
    print('student: ' + netid)
    os.chdir(args.folderLocation.replace('{NETID}', netid))
    with open('capture_script.stdout', 'w') as f:
        with open('capture_script.stderr', 'w') as g:
            subprocess.run([absolute_script_path], stdout = f, stderr = g)
    os.chdir(current_location)
