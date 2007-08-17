/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "encoder.h"

#include "prefs.h"
#include "encoder_prefs.h"
#include "ui_encoderoutput.h"

#include <QRegExp>
#include <QDir>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kdebug.h>
#include <knotification.h>
#include <q3textedit.h>
#include <kinputdialog.h>

class EncoderOutput : public QDialog, public Ui::EncoderOutput
{
public:
	EncoderOutput(QWidget* parent, const char* name)
	  : QDialog(parent)
	{
		setObjectName(name);
		setupUi(this);
	}
} ;

/**
 * Constructor, load settings.
 */
Encoder::Encoder( QObject* parent):QObject(parent),reportCount(0) {
	loadSettings();
}

/**
 * Load the settings for this class.
 */
void Encoder::loadSettings() {
	loadEncoder(Prefs::currentEncoder());
	// If the cpu count change then try
	for(uint i=0; i<(uint)Prefs::numberOfCpus(); i++)
		tendToNewJobs();
}

EncoderPrefs* Encoder::loadEncoder( int encoder ){
	EncoderPrefs* prefs;
	QString currentEncoderGroup = QString("Encoder_%1").arg(encoder);
	prefs = EncoderPrefs::prefs(currentEncoderGroup);
	if ( !EncoderPrefs::hasPrefs(currentEncoderGroup) ) {
		KMessageBox::sorry(0, i18n("No encoder has been selected.\nPlease select an encoder in the configuration."), i18n("No Encoder Selected"));
		prefs->setCommandLine(QString::null);
	}

	return prefs;
}

/**
 * Deconstructor, remove pending jobs, remove current jobs.
 */
Encoder::~Encoder() {
	qDeleteAll(pendingJobs);
	pendingJobs.clear();

	QMap<K3ShellProcess*, Job*>::Iterator pit;
	for( pit = jobs.begin(); pit != jobs.end(); ++pit ) {
		Job *job = pit.value();
		K3ShellProcess *process = pit.key();
		threads.removeAll(process);
		process->kill();
		QFile::remove(job->newLocation);
		delete job;
		delete process;
	}
	jobs.clear();
}

/**
 * @return The number of active jobs
 */
int Encoder::activeJobCount() {
	return jobs.count();
}

/**
 * @return The number of pending jobs
 */
int Encoder::pendingJobCount() {
	return pendingJobs.count();
}

/**
 * Stop this job with the matching id.
 * @param id the id number of the job to stop.
 */
void Encoder::removeJob(int id ) {
	QMap<K3ShellProcess*, Job*>::Iterator it;
	for( it = jobs.begin(); it != jobs.end(); ++it ) {
		if ( it.value()->id == id ) {
			K3ShellProcess *process = it.key();
			Job *job = it.value();
			threads.removeAll(process);
			process->kill();
			jobs.remove(process);
			delete job;
			delete process;
			break;
		}
	}
	Job *job = 0;
	foreach(Job* j, pendingJobs)
	{
		if ( j->id == id)
		{
			job = j;
			break;
		}
	}
	if ( job ) {
		pendingJobs.removeAll(job);
		delete job;
	}
	tendToNewJobs();
}

/**
 * Adds job to the que of jobs to encode.
 * @param job the job to encode.
 */
void Encoder::encodeWav(Job *job ) {
	emit(addJob(job, i18n("Encoding (%1): %2 - %3", loadEncoder(job->encoder)->extension(),
	                                                job->track_artist, job->track_title)));
	pendingJobs.append(job);
	tendToNewJobs();
}

/**
 * See if there are are new jobs to attend too.	If we are all loaded up
 * then just loop back in a few seconds and check agian.
 */
void Encoder::tendToNewJobs() {
	if ( pendingJobs.count() == 0 ) {
		emit jobsChanged();
		return;
	}

	// If we are currently ripping the max try again in a little bit.
	if ( (int)threads.count() >= Prefs::numberOfCpus() ) {
		emit jobsChanged();
		return;
	}

	Job *job = pendingJobs.takeFirst();

	EncoderPrefs* prefs = loadEncoder(job->encoder);

	QString desiredFile = Prefs::fileFormat();
	desiredFile.replace( QRegExp("~"), QDir::homePath() );
	{
		QHash <QString,QString> map;
		map.insert("extension", prefs->extension());
		Job jobx = *job;
		jobx.fix(Prefs::replaceInput(), Prefs::replaceOutput());
		jobx.fix("/", "%2f");
		// If the user wants anything regexp replaced do it now...
		desiredFile = jobx.replaceSpecialChars(desiredFile, false, map);
	}

	while ( QFile::exists( desiredFile ) ) {
		bool ok;
		QString text = KInputDialog::getText(
			i18n("File Already Exists"), i18n("Sorry, file already exists. Please pick a new name:"),
			desiredFile, &ok );
		if ( ok && !text.isEmpty() )
		 	desiredFile = text;
		else {
			emit jobsChanged();
			updateProgress(job->id, -1);
			return;
		}
	}

	int lastSlash = desiredFile.lastIndexOf('/',-1);
	if ( lastSlash == -1 ||
			!(KStandardDirs::makeDir( desiredFile.mid(0,lastSlash))) ) {
		KMessageBox::sorry(0, i18n("Cannot place file, unable to make directories."), i18n("Encoding Failed"));
		emit jobsChanged();
		updateProgress(job->id, -1);
		return;
	}

	job->newLocation = desiredFile;
	reportCount = 0;

	QString command = prefs->commandLine(); {
		QHash <QString,QString> map;
		map.insert("extension", prefs->extension());
		map.insert("f", job->location);
		map.insert("o", desiredFile);
		command = job->replaceSpecialChars(command, true, map);
	}

	updateProgress(job->id, 1);

	job->errorString = command;
	K3ShellProcess *proc = new K3ShellProcess();
	proc->setPriority(Prefs::niceLevel());

	*proc << QFile::encodeName(command);
	connect(proc, SIGNAL(receivedStdout(K3Process *, char *, int )),
		    this, SLOT(receivedThreadOutput(K3Process *, char *, int )));
	connect(proc, SIGNAL(receivedStderr(K3Process *, char *, int )),
		    this, SLOT(receivedThreadOutput(K3Process *, char *, int )));
	connect(proc, SIGNAL(processExited(K3Process *)), this, SLOT(jobDone(K3Process *)));
	jobs.insert(proc, job);
	threads.append(proc);

	proc->start(K3ShellProcess::NotifyOnExit, K3ShellProcess::AllOutput);
	emit jobsChanged();
}

/**
 * We have received some output from a thread. See if it contains %.
 * @param proc the process that has new output.
 * @param buffer the output from the process
 * @param buflen the length of the buffer.
 */
void Encoder::receivedThreadOutput(K3Process *process, char *buffer, int length ) {
	if ( Prefs::fullDecoderDebug() && buffer)
		kDebug(60002) << buffer;

	// Make sure we have a job to send an update too.
	if(jobs.find((K3ShellProcess*)process) == jobs.end()){
		kDebug(60002) << "Encoder::receivedThreadOutput Job doesn't exists. Line: " <<  __LINE__;
		return;
	}

	Job *job = jobs[(K3ShellProcess*)process];

	// Keep the output in the event it fails.
	job->output += QString(buffer).mid(0,length);

	// Make sure the output string has a % symble in it.
	QString output = QString(buffer).mid(0,length);
	if ( !output.contains('%') && reportCount < 5 ) {
		kDebug(60002) << "No \'%%\' in output. Report as bug w/encoder options if progressbar doesn't fill.";
		reportCount++;
		return;
	}
	//qDebug(QString("Pre cropped: %1").arg(output).latin1());
	output = output.mid(output.indexOf('%')-loadEncoder(job->encoder)->percentLength(),2);
	//qDebug(QString("Post cropped: %1").arg(output).latin1());
	bool conversionSuccessfull = false;
	int percent = output.toInt(&conversionSuccessfull);
	//qDebug(QString("number: %1").arg(percent).latin1());
	if ( percent >= 0 && percent < 100 && conversionSuccessfull ) {
		emit(updateProgress(job->id, percent));
	}
	// If it was just some random output that couldn't be converted then don't report the error.
	else
		if ( conversionSuccessfull )
			kWarning("Percent done:\"%d\" is not >= 0 && < 100.", percent);
}

/**
 * When the process is done encoding a file this function is called.
 * @param job the job that just finished.
 */
void Encoder::jobDone(K3Process *process ) {
	// Normal error checking here.
	if ( !process)
		return;

	//qDebug("Process exited with status: %d", process->exitStatus());

	Job *job = jobs[(K3ShellProcess*)process];
	threads.removeAll((K3ShellProcess*)process);
	jobs.remove((K3ShellProcess*)process);

	bool showDebugBox = false;
	if ( process->exitStatus() == 127 ) {
		KMessageBox::sorry(0, i18n("The selected encoder was not found.\nThe wav file has been removed. Command was: %1", job->errorString), i18n("Encoding Failed"));
		emit(updateProgress(job->id, -1));
	}
	else if ( QFile::exists(job->newLocation) ) {
		// fyi segfaults return 136
		if ( process->exitStatus() != 0 ) {
			if ( KMessageBox::questionYesNo(0, i18n("The encoder exited with a error.  Please check that the file was created.\nDo you want to see the full encoder output?"), i18n("Encoding Failed"),KGuiItem(i18n("Show Output")),KGuiItem(i18n("Skip Output"))) == KMessageBox::Yes )
			{
				showDebugBox = true;
			}
		}
		else{
			//qDebug("Must be done: %d", (process->exitStatus()));
			emit(updateProgress(job->id, 100));
			KNotification::event("track encoded");
			if ( job->lastSongInAlbum)
				KNotification::event("cd encoded");
		}
	}
	else
	{
		if ( KMessageBox::questionYesNo(0, i18n("The encoded file was not created.\nPlease check the encoder options.\nThe wav file has been removed.\nDo you want to see the full encoder output?"), i18n("Encoding Failed"),KGuiItem(i18n("Show Output")),KGuiItem(i18n("Skip Output"))) == KMessageBox::Yes )
		{
			showDebugBox = true;
		}
		emit( updateProgress( job->id, -1 ) );
	}

	if ( job->removeTempFile )
		QFile::remove( job->location );

	if( showDebugBox ){
		EncoderOutput dlg( 0, "Encoder Output" );
		job->output = job->errorString + "\n\n\n" + job->output;
		dlg.output->setText(job->output);
		dlg.exec();
	}

	delete job;
	delete process;
	tendToNewJobs();
}

#include "encoder.moc"

