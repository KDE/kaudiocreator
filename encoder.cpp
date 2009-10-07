/**
 * Copyright (C) 2003-2005 Benjamin C Meyer (ben at meyerhome dot net)
 * Copyright (C) 2009 Gerd Fleischer (gerdfleischer at web dot de)
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
#include <kinputdialog.h>
#include <kshell.h>
#include <kio/renamedialog.h>

#include <kdebug.h>


class EncoderOutput : public QDialog, public Ui::EncoderOutput
{
public:
	EncoderOutput(QWidget *parent = 0)
	  : QDialog(parent)
	{
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
	loadEncoder(Prefs::defaultEncoder());
	// If the cpu count change then try
	for(uint i=0; i<(uint)Prefs::numberOfCpus(); ++i)
		tendToNewJobs();
}

EncoderPrefs *Encoder::loadEncoder(QString encoder)
{
	EncoderPrefs *prefs = 0;
	QString currentEncoderGroup = QString("Encoder_").append(encoder);
	prefs = EncoderPrefs::prefs(currentEncoderGroup);
	if ( !EncoderPrefs::hasPrefs(currentEncoderGroup) ) {
		KMessageBox::sorry(0, i18n("No encoder has been selected.\nPlease select an encoder in the configuration."), i18n("No Encoder Selected"));
		prefs->setCommandLine(QString());
	}
	return prefs;
}

/**
 * Deconstructor, remove pending jobs, remove current jobs.
 */
Encoder::~Encoder()
{
	qDeleteAll(pendingJobs);
	pendingJobs.clear();

	QMap<KProcess *, Job *>::Iterator pit;
	for ( pit = jobs.begin(); pit != jobs.end(); ++pit ) {
		Job *job = pit.value();
		KProcess *process = pit.key();
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
int Encoder::activeJobCount()
{
	return jobs.count();
}

/**
 * @return The number of pending jobs
 */
int Encoder::pendingJobCount()
{
	return pendingJobs.count();
}

/**
 * Stop this job with the matching id.
 * @param id the id number of the job to stop.
 */
void Encoder::removeJob(int id )
{
	QMap<KProcess *, Job *>::Iterator it;
	for( it = jobs.begin(); it != jobs.end(); ++it ) {
		if ( it.value()->id == id ) {
			KProcess *process = it.key();
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
	foreach(Job *j, pendingJobs)
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
void Encoder::encodeWav(Job *job )
{
	emit(addJob(job, i18n("Encoding (%1): %2 - %3", loadEncoder(job->encoder)->extension(),
	                                                job->track_artist, job->track_title)));
	pendingJobs.append(job);
	tendToNewJobs();
}

/**
 * See if there are new jobs to attend too. If we are all loaded up
 * then just loop back in a few seconds and check agian.
 */
void Encoder::tendToNewJobs()
{
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

	QHash <QString,QString> map;
	map.insert("extension", prefs->extension());
	Job jobx = *job;
	jobx.fix(Prefs::replaceInput(), Prefs::replaceOutput());
	jobx.fix("/", "%2f");
	// If the user wants anything regexp replaced do it now...
	desiredFile = jobx.replaceSpecialChars(desiredFile, false, map, true);

	if (QFile::exists(desiredFile)) {
		KUrl desiredFileUrl = KUrl::fromPath(desiredFile);
		KIO::RenameDialog over(0, i18n("File Already Exists"), KUrl(), desiredFileUrl, KIO::M_OVERWRITE);
		int result = over.exec();
		switch (result) {
			case KIO::R_OVERWRITE:
				// keep the filename, overwriting existing files may need extra external encoder options, delete it here?
				break;
			case KIO::R_RENAME:
				desiredFileUrl = over.newDestUrl();
				desiredFile = desiredFileUrl.path();
				break;
			case KIO::R_CANCEL:
			default:
				emit jobsChanged();
				updateProgress(job->id, -1);
				return;
		}
	}

	int lastSlash = desiredFile.lastIndexOf('/',-1);
	if ( lastSlash == -1 ||
			!(KStandardDirs::makeDir( desiredFile.mid(0,lastSlash), 0775)) ) {
		KMessageBox::sorry(0, i18n("Cannot place file, unable to make directories."), i18n("Encoding Failed"));
		emit jobsChanged();
		updateProgress(job->id, -1);
		return;
	}

	job->newLocation = desiredFile;
	reportCount = 0;

	QString command = prefs->commandLine();
	map.clear();
	map.insert("extension", prefs->extension());
	map.insert("f", job->location);
	map.insert("o", desiredFile);

	int niceLevel = Prefs::niceLevel();
	QString niceProg = KStandardDirs::findExe("nice");
	if (niceLevel && niceProg != QString())
		command = niceProg + " -n " + QString::number(niceLevel) + " " + command;

	command = job->replaceSpecialChars(command, true, map);

	updateProgress(job->id, 0);
	job->errorString = command;

	EncodeProcess *proc = new EncodeProcess();
//	proc->setPriority(Prefs::niceLevel());

//	*proc << QFile::encodeName(command);
	connect(proc, SIGNAL(newEncodeOutput(EncodeProcess *)),
		    this, SLOT(receivedThreadOutput(EncodeProcess *)));
//	connect(proc, SIGNAL(receivedStderr(K3Process *, char *, int )),
//		    this, SLOT(receivedThreadOutput(KProcess *, char *, int )));
	connect(proc, SIGNAL(encodingFinished(KProcess *, int, QProcess::ExitStatus)),
		this, SLOT(jobDone(KProcess *)));
	jobs.insert(proc, job);
	threads.append(proc);

	proc->setOutputChannelMode(KProcess::MergedChannels);
    proc->setEnvironment(proc->systemEnvironment());
	proc->setProgram(KShell::splitArgs(command));
	proc->start();
	emit jobsChanged();
}

/**
 * We have received some output from a thread. See if it contains %.
 * @param proc the process that has new output.
 */
void Encoder::receivedThreadOutput(EncodeProcess *process) {
// 	if ( Prefs::fullDecoderDebug() && buffer)
// 		kDebug(60002) << buffer;

	// Make sure we have a job to send an update too.
	if(jobs.find((KProcess *)process) == jobs.end()){
		kDebug() << "Encoder::receivedThreadOutput Job doesn't exists. Line: " <<  endl;
		return;
	}

	Job *job = jobs[(KProcess *)process];

	QByteArray outputArray = process->readAllStandardOutput();
	QString output = QString(outputArray);
	// Keep the output in the event it fails.
	job->output += output;

	// Make sure the output string has a % symble in it.
	if ( !output.contains('%') && reportCount < 5 ) {
		kDebug() << "No \'%%\' in output. Report as bug w/encoder options if progressbar doesn't fill." << endl;
		reportCount++;
		return;
	}
	//qDebug(QString("Pre cropped: %1").arg(output).latin1());
	output = output.mid(output.indexOf('%')-loadEncoder(job->encoder)->percentLength(),2);
	//qDebug(QString("Post cropped: %1").arg(output).latin1());
	bool conversionSuccessfull = false;
	int percent = output.toInt(&conversionSuccessfull);
	//qDebug(QString("number: %1").arg(percent).latin1());
	if ( percent >= 0 && percent < JOB_COMPLETED && conversionSuccessfull ) {
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
void Encoder::jobDone(KProcess *process)
{
	kDebug() << "jobDone" << endl;
	// Normal error checking here.
	if ( !process)
		return;

	//qDebug("Process exited with status: %d", process->exitStatus());

	Job *job = jobs[(KProcess *)process];
	EncoderPrefs *encPrefs = loadEncoder(job->encoder);
	threads.removeAll((KProcess *)process);
	jobs.remove((KProcess *)process);
	bool showDebugBox = false;
	if ( process->exitCode() == 127 ) {
		KMessageBox::sorry(0, i18n("The selected encoder was not found.\nThe wav file has been removed. Command was: %1", job->errorString), i18n("Encoding Failed"));
		emit(updateProgress(job->id, JOB_ERROR));
	} else if (encPrefs->checkOutput() && encPrefs->commandLine().contains("%o") && QFile::exists(job->newLocation)) {
		// fyi segfaults return 136
		if ( process->exitCode() != 0 ) {
			if ( KMessageBox::questionYesNo(0, i18n("The encoder exited with a error.  Please check that the file was created.\nDo you want to see the full encoder output?"), i18n("Encoding Failed"),KGuiItem(i18n("Show Output")),KGuiItem(i18n("Skip Output"))) == KMessageBox::Yes )
			{
				showDebugBox = true;
			}
			emit( updateProgress( job->id, JOB_ERROR) );
		} else {
			//qDebug("Must be done: %d", (process->exitStatus()));
			emit(updateProgress(job->id, JOB_COMPLETED));
			KNotification::event("track encoded");
			if ( job->lastSongInAlbum)
				KNotification::event("cd encoded");
		}
	} else if (!encPrefs->checkOutput()) {
		emit(updateProgress(job->id, JOB_COMPLETED));
		KNotification::event("track encoded");
		if ( job->lastSongInAlbum)
			KNotification::event("cd encoded");
	} else {
		if ( KMessageBox::questionYesNo(0, i18n("The encoded file was not created.\nPlease check the encoder options.\nThe wav file has been removed.\nDo you want to see the full encoder output?"), i18n("Encoding Failed"),KGuiItem(i18n("Show Output")),KGuiItem(i18n("Skip Output"))) == KMessageBox::Yes )
		{
			showDebugBox = true;
		}
		emit( updateProgress( job->id, JOB_ERROR) );
	}

	if ( job->removeTempFile )
		QFile::remove( job->location );

	if( showDebugBox ){
		EncoderOutput dlg;
		job->output = job->errorString + "\n\n\n" + job->output;
		dlg.output->setText(job->output);
		dlg.exec();
	}

	delete job;
	delete process;
	tendToNewJobs();
}

#include "encoder.moc"

