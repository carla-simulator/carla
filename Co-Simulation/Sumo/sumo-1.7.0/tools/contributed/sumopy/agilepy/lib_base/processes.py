# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
# SUMOPy module
# Copyright (C) 2012-2017 University of Bologna - DICAM
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    processes.py
# @author  Joerg Schweizer
# @date


import subprocess
import os
import sys
import types


#SUMOPYDIR = os.path.join(os.path.dirname(__file__),"..")
#TOOLSDIR = os.path.join(SUMOPYDIR,"..")

#APPDIR =  os.path.join(os.path.dirname(__file__),"..")


import classman as cm
from misc import filepathlist_to_filepathstring, filepathstring_to_filepathlist, ff, P


# p = psutil.Process(the_pid_you_want) !!
# print p.status
# if p.status == psutil.STATUS_ZOMBIE:

##processlist = psutil.get_process_list()
##
# for p in processlist:
# if psutil.pid_exists(p.pid):
# print '  pid = ',p.pid
# print '    name =',p.name
# print '    running =',p.is_running()
# print '    path =',p.path
# print '    cmdline',p.cmdline
##
##
##cmd = 'nohup sumo --no-step-log --no-duration-log --no-warnings --step-length %.3f -c %s'%(time_simstep, filepath_netconfig)
##
# print '\n Starting command:',cmd
##p = subprocess.Popen(cmd, shell=True)
# print "p.pid",p.pid, "p.poll=",p.poll()
# p.wait()
# print '\n\nreturncode',p.poll(),p.returncode
# print 'p.stdout=\n', p.stdout
# call(cmd)

# in windows : start command3


def call(cmd):
    """
    Calls a sytem commend lime. Returns 1 if executed successfully.
    """
    # ensure unix compatibility
    print(cmd)
    # if isinstance(cmd, str):
    #    cmd = filter(lambda a: a!='', cmd.split(' '))
    # subprocess.call(cmd)
    p = subprocess.Popen(cmd, shell=True)
    p.wait()
    return p.returncode == 0


class Process(cm.BaseObjman):
    def __init__(self, ident,  **kwargs):
        self._init_common(ident, **kwargs)

    def _init_common(self,  ident, parent=None, name=None, **kwargs):
        self._init_objman(ident=ident, parent=parent, name=name, **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))
        #self.net = attrsman.add(   cm.ObjConf( network.Network(self) ) )
        self.status = attrsman.add(cm.AttrConf(
            'status', 'preparation',
            groupnames=['_private', 'parameters'],
            perm='r',
            name='Status',
            info='Process status: preparation-> running -> success|error.'
        ))
        self._logger.w(self.get_name(), key='action')
        self._logger.w('Prepare', key='message')

    def _set_status(self, status):
        self.status = status

    def get_status(self):
        return self.status

    def get_kwoptions(self):
        return self.get_attrsman().get_group_attrs('options')

    def run(self):
        if self.is_ready():
            logger = self.get_logger()
            self.status = 'running'
            logger.start('Start process: %s' % self.get_name())

            #logger.w('start', key='message')
            #
            ans = self.do()
            #
            #logger.w(self.status, key='message')
            if ans == True:
                self.status = 'success'
            else:
                self.status = 'error'
            logger.stop('Finished %s with status %s.' % (self.get_name(), self.status))

            return self.status == 'success'
            # f self.status == 'error':
            #    return True
            # self.status = 'success' # self.status = 'error'
            # return True
        else:
            logger.stop('Cannot start %s.' % (self.get_name(),))
            return False

    def aboard(self):
        self.status = 'aboarded'

    def do(self):
        #self.status = 'success'
        return True

    def update_params(self):
        """
        Make all parameters consistent.
        example: used by import OSM to calculate/update number of tiles
        from process dialog
        """
        pass

    def is_ready(self):
        """
        Returns True if process is ready to run.

        """
        return True


class Options:
    """
    Dummy option class to fake option parsing
    """

    def __init__(self, **kwargs):
        self._optionattrs = []
        self._transdir = {}
        self._filepathattrs = []
        for attr, value in kwargs.iteritems():
            self.add_option(attr, value)

    def add_option(self, attr='', value='', cml=None, is_filepath=False):
        setattr(self, attr, value)
        self._optionattrs.append(attr)
        if cml is not None:
            self._transdir[attr] = cml
        if is_filepath:
            self._filepathattrs.append(attr)

    def set_transdir(self, **transdir):
        """
        Sets a dictionary to translate python compatible
        option names into the command line optionnames,
        only in case the command line options are not identical 
        with python attributes (for example if 
        command line options contain '.' or '-').
        Format of  transdir is python attribute as key and
        command line option (as string, WITH preceeding'--') as value.

        """
        self._transdir = transdir

    def get_optionstring(self):
        # print 'get_optionstring'
        s = ''
        for attr in self._optionattrs:
            value = getattr(self, attr)
            cmlattr = self._transdir.get(attr, attr)
            # print '  option',attr,cmlattr,attr in self._filepathattrs,type(value)
            if attr in self._filepathattrs:
                if value != '':
                    s += ' '+cmlattr+' %s' % filepathlist_to_filepathstring(value.split(','))
            elif type(value) == types.BooleanType:
                if value:
                    s += ' '+cmlattr
            elif type(value) in [types.StringTypes, types.UnicodeType]:
                if P == '"':  # windows
                    s += ' '+cmlattr+' "%s"' % value
                else:
                    s += ' '+cmlattr+" '%s'" % value
            else:
                s += ' '+cmlattr+' %s' % value
        return s


class CmlMixin:
    def init_cml(self, command, is_run_background=False, is_nohup=False):
        self.optiongroupname = 'cml-options'
        attrsman = self.get_attrsman()
        self.pathmetatypes = ['filepath', 'dirpath', 'filepaths', 'dirpaths']
        self._command = attrsman.add(cm.AttrConf(
            '_command', command,
            groupnames=['_private'],
            perm='r',
            name='command',
            info='Command to be executed.'
        ))
        self.pid = attrsman.add(cm.AttrConf(
            'pid', -1,
            groupnames=['_private'],
            perm='r',
            name='Process ID',
            info="The system's Process ID",
        ))

        self.is_run_background = attrsman.add(cm.AttrConf(
            'is_run_background', is_run_background,
            groupnames=['parameters', 'advanced'],
            perm='rw',
            name='Run in background',
            info='If set, process will run in background.',
        ))

        self.is_nohup = attrsman.add(cm.AttrConf(
            'is_nohup', is_nohup,
            groupnames=['parameters', 'advanced', ],
            perm='rw',
            name='No hangup',
            info="""If set, process will run in the background and will continue to run after logout. (Currently on UNIX platforms only.) """,
        ))

    def add_option(self, option, value, **kwargs):
        kwargs0 = {'cml': None,
                   'groupnames': [],
                   'perm': 'rw',
                   'is_save': True,
                   'name': None,
                   'info': '',
                   }

        kwargs0.update(kwargs)
        if not (self.optiongroupname in kwargs0['groupnames']):
            kwargs0['groupnames'] += [self.optiongroupname]

        # print '\nadd_option', option, value,kwargs0
        default = self.get_attrsman().add(cm.AttrConf(option, value, **kwargs0))
        setattr(self, option, default)

    def get_options(self):
        print '\nget_options'
        options = Options()
        for attrconfig in self.get_attrsman().get_configs(is_all=True):
            if self.optiongroupname in attrconfig.groupnames:
                print '  option', attrconfig.attrname, attrconfig.groupnames, attrconfig.get_metatype() in self.pathmetatypes
                is_enabled = True
                if hasattr(attrconfig, 'is_enabled'):
                    # print ' is_enabled=',attrconfig.is_enabled(self), attrconfig.get_value()
                    is_enabled = attrconfig.is_enabled(self)
                if is_enabled:  # disabeled options are simply not added
                    is_filepath = attrconfig.get_metatype() in self.pathmetatypes
                    options.add_option(attrconfig.attrname, attrconfig.get_value(),
                                       attrconfig.cml, is_filepath=is_filepath)

        return options

    def print_options(self):
        print 'Options of process ident:', self.ident
        print ' Keywordoptions:'
        for attrconfig in self.get_attrsman().get_configs(filtergroupnames=[self.optiongroupname]):
            print '  ', attrconfig.attrname, '=', attrconfig.get_value()

    def reset_cml(self, cml):
        self._command = cml

    def get_cml(self, is_changecwd=False):
        """
        Returns commandline with all options.
        To be overridden by costum class.
        """
        options = self.get_options()

        if is_changecwd:
            if self.get_workdirpath() is None:
                cwd = ''
            else:
                cwd = 'cd '+self.get_workdirpath()+' ;'
        else:
            cwd = ''
        return cwd+self._command + options.get_optionstring()

    def run_cml(self, cml=None):
        if cml is None:
            cml = self.get_cml()
        attrsman = self.get_attrsman()
        self._subprocess = subprocess.Popen(cml, shell=True)
        attrsman.pid.set(self._subprocess.pid)
        attrsman.status.set('running')
        print 'run_cml cml=', cml
        print '  pid = ', self.pid
        if not self.is_run_background:
            self._subprocess.wait()

            if self._subprocess.returncode == 0:
                attrsman.status.set('success')
                return True

            else:
                attrsman.status.set('error')
                return False


class ProcessOld(cm.BaseObjman):
    def __init__(self, ident, command=None, parent=None, name=None,
                 is_inputfilelist=True, is_outputfilelist=True,
                 is_force=False, is_run_background=False, is_nohup=False,
                 workdirpath=None):
        self._init_objman(ident, parent=parent, name=name)
        self.attrs = self.set_attrman(cm.AttrsManager(self, 'attrs'))

        if command is None:
            command_default = ''
        else:
            command_default = command

        self.set_workdirpath(workdirpath)

        self.status = self.attrs.add(cm.AttrConf(
            'status', 'preparation',
            groupnames=['parameters'],
            perm='r',
            metatype='internal',
            name='Status',
            info='Process status: preparation-> running -> success|error.'
        ))

        self._command = self.attrs.add(cm.AttrConf(
            '_command', command_default,
            groupnames=['parameters', '_private', 'advanced'],
            perm='r',
            metatype='internal',
            name='command',
            info='Command to be executed.'
        ))

        self.progress = self.attrs.add(cm.AttrConf(
            'progress', 0.0,
            groupnames=['parameters'],
            perm='r',
            metatype='progress',
            unit='%',
            name='Progress',
            info='Indicates the percentage of completion of the process.'
        ))

        self.pid = self.attrs.add(cm.AttrConf(
            'pid', -1,
            groupnames=['parameters', 'advanced'],
            perm='r',
            name='Process ID',
            info="The system's Process ID",
        ))

        self.is_force = self.attrs.add(cm.AttrConf(
            'is_force', is_force,
            groupnames=['parameters', 'advanced'],
            perm='rw',
            name='is_force',
            info='If set, process will be executed even if output files exist.',
        ))

        self.is_run_background = self.attrs.add(cm.AttrConf(
            'is_run_background', is_run_background,
            groupnames=['parameters', 'advanced'],
            perm='rw',
            name='Run in background',
            info='If set, process will run in background.',
        ))

        self.is_nohup = self.attrs.add(cm.AttrConf(
            'is_nohup', is_nohup,
            groupnames=['parameters', 'advanced'],
            perm='rw',
            name='No hangup',
            info="""If set, process will run in the background and will continue to run after logout. (Currently on UNIX platforms only.) """,
        ))

        if is_inputfilelist:
            self.files_input = self.add_tableman(cm.TableManager(ident='files_input',
                                                                 parent=self,
                                                                 name='Input files',
                                                                 is_keyindex=True),
                                                 )

            self.files_input.add(cm.DictConf('filename', '',
                                             groupnames=['input'],
                                             perm='rw',
                                             name='Name',
                                             info='File name of input file',
                                             ))

            self.files_input.add(cm.DictConf('is_existent', False,
                                             groupnames=['output'],
                                             perm='r',
                                             name='Exists?',
                                             info='If set, this input file is exists.',
                                             ))

            self.files_input.add(cm.DictConf('is_required', True,
                                             groupnames=['output'],
                                             perm='r',
                                             name='Required?',
                                             info='If set, this input file is required.',
                                             ))

            self.files_input.add(cm.DictConf('filepath', '',
                                             groupnames=['input'],
                                             perm='rw',
                                             metatype='filepath',
                                             name='Path',
                                             info='Filepath of input file',
                                             ))

            self.files_input.add(cm.DictConf('fileoption', '',
                                             groupnames=['input', 'advanced'],
                                             perm='r',
                                             name='Option',
                                             info='Command line option',
                                             ))

            self.files_input.add(cm.DictConf('fileinfo', '',
                                             groupnames=['input', 'advanced'],
                                             perm='rw',
                                             name='Info',
                                             info='File info  of input file',
                                             ))
            self.files_input.add(cm.DictConf('wildcards', '',
                                             groupnames=['input', 'advanced'],
                                             perm='rw',
                                             name='wildcards',
                                             info='Wildcards for file filtering',
                                             ))

        if is_outputfilelist:
            self.files_output = self.add_tableman(cm.TableManager(
                ident='files_output',
                parent=self,
                name='Output files',
                is_keyindex=True),
            )

            self.files_output.add(cm.DictConf('filename', '',
                                              groupnames=['output'],
                                              perm='rw',
                                              name='Name',
                                              info='File name  of output file',
                                              ))

            self.files_output.add(cm.DictConf('filepath', '',
                                              groupnames=['output'],
                                              perm='rw',
                                              metatype='filepath',
                                              name='Path',
                                              info='Filepath of output file',
                                              ))

            self.files_output.add(cm.DictConf('fileoption', '',
                                              groupnames=['advanced'],
                                              perm='r',
                                              name='Option',
                                              info='Command line option',
                                              ))

            self.files_output.add(cm.DictConf('fileinfo', '',
                                              groupnames=['output', 'advanced'],
                                              perm='r',
                                              is_save=True,
                                              name='Info',
                                              info='File info of output file',
                                              ))

            self.files_output.add(cm.DictConf('wildcards', '',
                                              groupnames=['output', 'advanced'],
                                              perm='rw',
                                              is_save=True,
                                              name='wildcards',
                                              info='Wildcards for file filtering',
                                              ))

    def add_option(self, option, value, **kwargs):
        """
        option = string with option 
        value = value of option
        name = human readable name
        info = help info
        cml = string to be specified if command line string different from option
        """
        kwargs0 = {'cml': None,
                   'groupnames': [],
                   'perm': 'rw',
                   'is_save': True,
                   'name': None,
                   'info': '', }

        kwargs0.update(kwargs)
        if not ('options' in kwargs0['groupnames']):
            kwargs0['groupnames'] += ['options']

        default = self.attrs.add(cm.AttrConf(option, value, **kwargs0))

        setattr(self, option, default)

    def add_inputfile(self, filekey, filepath, is_required=True,
                      cml=None, name=None, info='', wildcards=''):
        self.files_input.add_row(key=filekey)
        self.files_input.filepath.set(filekey, filepath)
        self.files_input.is_existent.set(filekey, self.is_inputfile(filekey))
        self.files_input.is_required.set(filekey, is_required)
        self.files_input.fileoption.set(filekey, cml)
        self.files_input.filename.set(filekey, name)
        self.files_input.fileinfo.set(filekey, info)
        self.files_input.wildcards.set(filekey, wildcards)

    def add_outputfile(self, filekey, filepath,
                       cml=None, name=None, info='', wildcards=''):
        self.files_output.add_row(key=filekey)
        self.files_output.filepath.set(filekey, filepath)
        self.files_output.fileoption.set(filekey, cml)
        self.files_output.filename.set(filekey, name)
        self.files_output.fileinfo.set(filekey, info)
        self.files_output.wildcards.set(filekey, wildcards)

    def set_workdirpath(self, workdirpath):
        """
        Set  working directory for process.
        If this directory is set then processes will be run
        in this path and outputs without explicit path
        will end in this directory.
        Moreover, all filenames withot explicit path (absolue or relative)
        are supposed to be in this directory.
        """
        self._workdirpath = workdirpath

    def get_workdirpath(self):
        return self._workdirpath

    def set_path_inputfile(self, filekey, filepaths):
        self.files_input.filepath.set(filekey, filepaths)
        self.files_input.is_existent.set(filekey, self.is_inputfile(filekey))

    def set_path_outputfile(self, filekey, filepaths):
        self.files_output.filepath.set(filekey, filepaths)

    def get_path_inputfile(self, filekey):
        return self.files_input.filepath.get(filekey)

    def get_path_outputfile(self, filekey):
        return self.files_output.filepath.get(filekey)

    def get_name_inputfile(self, filekey):
        return os.path.basename(self.files_input.filepath.get(filekey))

    def get_name_outputfile(self, filekey):
        return os.path.basename(self.files_output.filepath.get(filekey))

    def set_existents_inputfiles(self):
        for filekey in self.files_input.get_keys():
            self.files_input.is_existent.set(filekey, self.is_inputfile(filekey))

    def get_inputfiles_missing(self):
        """
        Returns list with paths of all input files that 
        are required but not existent in filesystem
        """
        files_missing = []
        if hasattr(self, 'files_input'):
            for filekey in self.files_input.get_keys():
                if (not self.is_inputfile(filekey)) & (self.files_input.is_required.get(filekey)):
                    files_missing.append(self.files_input.filepath.get(filekey))

        return files_missing

    def get_outputfiles_missing(self):
        """
        Returns list with paths of all output files that 
        are expected to be produced and are currently not existent in filesystem
        """
        # print '\n\nget_outputfiles_missing'
        files_missing = []
        if hasattr(self, 'files_output'):
            for filekey in self.files_output.get_keys():
                if not self.is_outputfile(filekey):
                    files_missing.append(self.files_output.filepath.get(filekey))

        return files_missing

    def is_inputfile(self, filekey):
        print 'is_inputfile', filekey

        filepaths = self.files_input.filepath.get(filekey)
        # print '  filepaths =',filepaths,type(filepaths)
        if type(filepaths) in types.StringTypes:
            # print '    call filepathstring_to_filepathlist'
            filepaths = filepathstring_to_filepathlist(filepaths)

        # print  '  filepaths list',filepaths
        if len(filepaths) > 0:
            ans = True
            for filepath in filepaths:
                dirpath = os.path.dirname(filepath)
                # this is to make sure that check always works
                # either with filename or with filepath
                if (self._workdirpath is not None) & (dirpath == ''):
                    filepath = os.path.join(self._workdirpath, filepath)
                print '  check is_inputfile: >>%s<< exists = %d' % (filepath, os.path.isfile(filepath))
                ans = ans & os.path.isfile(filepath)
            # print '  is_inputfile=',ans
            return ans
        else:
            return False  # no path given

    def is_outputfile(self, filekey):
        ans = True
        filepaths = self.files_output.filepath.get(filekey)
        # print '\nis_outputfile  filepaths = >%s<'%filepaths,type(filepaths)
        if type(filepaths) in types.StringTypes:
            # print '    call filepathstring_to_filepathlist'
            filepaths = filepathstring_to_filepathlist(filepaths)

        # print  '  filepaths list',filepaths
        for filepath in filepaths:
            dirpath = os.path.dirname(filepath)
            # this is to make sure that check always works
            # either with filename or with filepath
            if (self._workdirpath is not None) & (dirpath == ''):
                filepath = os.path.join(self._workdirpath, filepath)
            # print '  check is_outputfile: >>%s<< exists = %d'%(filepath,os.path.isfile(filepath))
            ans = ans & os.path.isfile(filepath)
        # print '  is_outputfile=',ans
        return ans

        #ans = True
        #filepaths = self.files_output.filepath.get(filekey)
        # for filepath in cm.filepathstring_to_filepathlist(filepaths):
        #    dirpath = os.path.dirname(filepath)
        #    if (self._workdirpath is not None)&(dirpath==''):
        #        filepath = os.path.join(self._workdirpath,filepath)
        #    #print '  check is_outputfile:',filepath,os.path.isfile(filepath)
        #    ans = ans & os.path.isfile(filepath)
        # return ans

    def update_params(self):
        """
        Make all parameters consistent.
        example: used by import OSM to calculate/update number of tiles
        from process dialog
        """
        pass

    def is_ready(self):
        """
        Returns True if process is ready to run.

        """
        if (self._command is not None) & (self._command != ''):
            # check if all input files exist
            return len(self.get_inputfiles_missing()) == 0

        else:
            print 'WARNING: command has not been set'
            return False

    def is_done(self):
        """
        Returns True if process produced all output files.
        """
        return len(self.get_outputfiles_missing()) == 0

    def get_options(self):
        options = Options()
        for attrconfig in self.attrs.get_configs():
            if 'cml-options' in attrconfig.groupnames:
                options.add_option(attrconfig.attrname, attrconfig.get_attr(), attrconfig.cml, is_filepath=False)

        if hasattr(self, 'files_input'):
            for filekey in self.files_input.get_keys():
                filepath = self.files_input.filepath.get(filekey)

                cml = self.files_input.fileoption.get(filekey)
                options.add_option(filekey, filepath.replace(',', ' '), cml, is_filepath=True)

        if hasattr(self, 'files_output'):
            for filekey in self.files_output.get_keys():
                filepath = self.files_output.filepath.get(filekey)
                cml = self.files_output.fileoption.get(filekey)
                options.add_option(filekey, filepath.replace(',', ' '), cml, is_filepath=True)

        return options

    def print_options(self):
        print 'Options of process ident:', self.ident
        print ' Keywordoptions:'
        for attrconfig in self.attrs.get_configs():
            if 'options' in attrconfig.groupnames:
                print '  ', attrconfig.attrname, '=', attrconfig.get_attr()

        print ' Input files:'
        if hasattr(self, 'files_input'):
            for filekey in self.files_input.get_keys():
                filepath = self.files_input.filepath.get(filekey)
                print '  ', filekey, '=', filepath

        print ' Output files:'
        if hasattr(self, 'files_output'):
            for filekey in self.files_output.get_keys():
                filepath = self.files_input.filepath.get(filekey)
                print '  ', filekey, '=', filepath

    def get_cml(self, is_changecwd=False):
        """
        Returns commandline with all options.
        To be overridden by costum class.
        """
        options = self.get_options()

        if is_changecwd:
            if self.get_workdirpath() is None:
                cwd = ''
            else:
                cwd = 'cd '+self.get_workdirpath()+' ;'
        else:
            cwd = ''
        return cwd+self._command + options.get_optionstring()

    def run(self):
        if self.is_ready():
            if (not self.is_done()) | self.is_force:
                cml = self.get_cml()
                self._subprocess = subprocess.Popen(cml, shell=True)
                self.attrs.pid.set(self._subprocess.pid)
                self.attrs.status.set('running')
                print 'run cml=', cml
                print '  pid = ', self.pid
                if not self.is_run_background:
                    self._subprocess.wait()

                    if self._subprocess.returncode == 0:
                        self.attrs.status.set('success')
                        return True

                    else:
                        self.attrs.status.set('error')
                        return False
            else:
                print 'WARNING in run: process has already generated output files.'
        else:
            print 'WARNING: process not ready to run.'
            return False

    def kill(self):
        pass


# print '\n Starting command:',cmd
##p = subprocess.Popen(cmd, shell=True)
# print "p.pid",p.pid, "p.poll=",p.poll()
# p.wait()
# print '\n\nreturncode',p.poll(),p.returncode
# print 'p.stdout=\n', p.stdout
# call(cmd)


###############################################################################
if __name__ == '__main__':
    """
    Test
    """

    # for testing only to get the object browser...
    sys.path.append(os.path.join(APPDIR, "lib_meta_wx"))
