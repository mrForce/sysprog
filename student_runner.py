import argparse
import os
import subprocess
parser = argparse.ArgumentParser(description='You pass student_runner five or six things: 1) A file with a list of NetIDs 2) A bash script to execute in the students folder 3) The location of the replay program  5) The location of the folder where the assignment is contained (using {NETID} as a placeholder for each person\'s NetID, and optionally 6) A folder whose contents should be copied to the assignment folder')
parser.add_argument('netids', help='A file containing a NetID on each line')
parser.add_argument('script', help='The script to run. ')
parser.add_argument('replay', help='location of the replay program')
parser.add_argument('entryFiles', help='The location of the folder containing the entry files')
parser.add_argument('assignmentFolderLocation', help='The location of the assignments folder (with {NETID} standing in for NetID)')
parser.add_argument('--CopyFiles', help='A folder whos contents will be copied to each assignment folder ')

args = parser.parse_args()
assert('{NETID}' in args.assignmentFolderLocation)
replay_location = os.path.abspath(args.replay)
assert(os.path.isfile(replay_location))
entry_files_location = os.path.abspath(args.entryFiles)
assert(os.path.isdir(entry_files_location))
netid_file_object = open(args.netids, 'r')
#just opening this to make sure it exists
script_file_object = open(args.script, 'r')
netids = []
no_assignment_folder_netids = []
for line in netid_file_object:
    if len(line) > 0:
        netid = line.strip()
        assignment_folder_location = args.assignmentFolderLocation
        student_location = assignment_folder_location.replace('{NETID}', netid)
        if os.path.isdir(student_location):
            netids.append(netid)
        else:
            print('No assignment folder for student: ' + netid)
            no_assignment_folder_netids.append(netid)
copy_files = []
if args.CopyFiles:
    assert(os.path.isdir(args.CopyFiles))
    for filename in os.listdir(args.CopyFiles):
        file_path = os.path.abspath(os.path.join(args.CopyFiles, filename))
        if os.path.isfile(file_path):
            copy_files.append(file_path)
absolute_script_path = os.path.abspath(args.script)
print('absolute script path: ' + absolute_script_path)
current_location = os.getcwd()
custom_env = os.environ.copy()
custom_env['REPLAY'] = replay_location
custom_env['ENTRY_FILE_BASE'] = entry_files_location
for netid in netids:
    print('student: ' + netid)
    assignment_folder_location = args.assignmentFolderLocation
    os.chdir(assignment_folder_location.replace('{NETID}', netid))
    print('current location: ' + os.getcwd())
    for x in copy_files:
        shutil.copy(x, os.getcwd())
    
    with open('capture_script.stdout', 'w') as f:
        with open('capture_script.stderr', 'w') as g:
            subprocess.run([absolute_script_path], env=custom_env, stdout = f, stderr = g)
    os.chdir(current_location)

for netid in no_assignment_folder_netids:
    print('No assignment folder for student with NetID: ' + netid)
