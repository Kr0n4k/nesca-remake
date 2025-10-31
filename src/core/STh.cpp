#include <STh.h>
#include <MainStarter.h>
#include <iostream>
#include <QTextStream>
#include <QString>
#include <QRegularExpression>

// ANSI color codes for terminal output
#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_RED     "\033[31m"
#define ANSI_BLUE    "\033[34m"
#define ANSI_CYAN    "\033[36m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_GRAY    "\033[90m"

// Progress bar update throttling (update max once per N calls)
static unsigned long g_lastProgressUpdate = 0;
static const unsigned long PROGRESS_UPDATE_INTERVAL = 50; // Update every 50 processed targets

// Helper function to strip HTML tags and decode HTML entities
QString stripHtmlTags(const QString& html) {
	QString result = html;
	
	// Remove script and style blocks completely (including their content)
	QRegularExpression scriptRegex("<script[^>]*>.*?</script>", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
	result.remove(scriptRegex);
	QRegularExpression styleRegex("<style[^>]*>.*?</style>", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
	result.remove(styleRegex);
	
	// Remove base64 image data (data:image/...)
	QRegularExpression base64Regex("data:image/[^;]*;base64,[^\"'\\s]*", QRegularExpression::CaseInsensitiveOption);
	result.remove(base64Regex);
	
	// Remove HTML tags
	QRegularExpression tagRegex("<[^>]+>");
	result.remove(tagRegex);
	
	// Decode common HTML entities
	result.replace("&lt;", "<");
	result.replace("&gt;", ">");
	result.replace("&amp;", "&");
	result.replace("&quot;", "\"");
	result.replace("&#39;", "'");
	result.replace("&nbsp;", " ");
	
	// Remove [PEKO] placeholder text if it's standalone
	result.replace("[PEKO]", "");
	result.replace("[R]", "");
	
	// Clean up extra whitespace and newlines
	result = result.trimmed();
	result.replace(QRegularExpression("\\s+"), " ");
	result.replace(QRegularExpression("\\n+"), " ");
	
	return result;
}

// Helper function to extract URL from HTML link
QString extractUrlFromHtml(const QString& html) {
	// First try to extract href attribute
	QRegularExpression urlRegex(R"(href=["']([^"']+)["'])");
	QRegularExpressionMatch match = urlRegex.match(html);
	if (match.hasMatch()) {
		QString url = match.captured(1);
		// Clean up URL if it ends with just /
		if (url.endsWith("/")) {
			url.chop(1);
		}
		return url;
	}
	
	// If no href, try to extract visible text from <a> tags
	QRegularExpression textRegex(R"(<a[^>]*>([^<]+)</a>)");
	QRegularExpressionMatch textMatch = textRegex.match(html);
	if (textMatch.hasMatch()) {
		return textMatch.captured(1).trimmed();
	}
	
	// Fallback: strip all HTML and return clean text
	return stripHtmlTags(html);
}

// Get human-readable CURL error description
QString getCurlErrorString(int curlCode) {
	switch (curlCode) {
		case 1: return "Unsupported protocol";
		case 5: return "Couldn't resolve proxy";
		case 6: return "Couldn't resolve host";
		case 7: return "Failed to connect";
		case 8: return "FTP weird server reply";
		case 18: return "Partial file transfer";
		case 28: return "Operation timeout";
		case 35: return "SSL connect error";
		case 47: return "Too many redirects";  // Not really an error for scanning
		case 52: return "Empty server response";
		case 56: return "Receive error";
		case 67: return "Login denied";
		default: return "Unknown error";
	}
}

// Консольные версии методов для вывода в stdout/stderr
//BA TablelistView
void STh::doEmitionChangeBARow(int index, QString loginPass, QString percentage)
{
	QTextStream out(stdout);
	QString cleanLoginPass = stripHtmlTags(loginPass);
	out << ANSI_BLUE << "[BA]" << ANSI_RESET << " " << cleanLoginPass 
	    << ANSI_GRAY << " (" << percentage << "%)" << ANSI_RESET << Qt::endl;
}

void STh::doEmitionShowRedVersion()
{
	QTextStream out(stdout);
	out << ANSI_RED << ANSI_BOLD << "[WARN] Version notification" << ANSI_RESET << Qt::endl;
}

void STh::doEmitionStartScanIP()
{
	QTextStream out(stdout);
	out << ANSI_BOLD << ANSI_CYAN << "[SCAN] Starting IP scan..." << ANSI_RESET << Qt::endl;
}

void STh::doEmitionStartScanDNS()
{
	QTextStream out(stdout);
	out << ANSI_BOLD << ANSI_CYAN << "[SCAN] Starting DNS scan..." << ANSI_RESET << Qt::endl;
}

void STh::doEmitionStartScanImport()
{
	QTextStream out(stdout);
	out << ANSI_BOLD << ANSI_CYAN << "[SCAN] Starting import scan..." << ANSI_RESET << Qt::endl;
}

void STh::doEmitionAddIncData(QString ip, QString str)
{
	QTextStream out(stdout);
	QString cleanStr = stripHtmlTags(str);
	out << ANSI_BLUE << "→" << ANSI_RESET << " " << ANSI_CYAN << ip << ANSI_RESET 
	    << ANSI_GRAY << ": " << ANSI_RESET << cleanStr << Qt::endl;
}

void STh::doEmitionAddOutData(QString str)
{
	QTextStream out(stdout);
	QString cleanStr = stripHtmlTags(str);
	out << ANSI_BLUE << "←" << ANSI_RESET << " " << cleanStr << Qt::endl;
}

void STh::doEmitionFoundData(QString str)
{
	QTextStream out(stdout);
	
	// Print newline after progress bar before outputting found data
	out << Qt::endl;
	
	// First extract URL from HTML
	QString url = extractUrlFromHtml(str);
	QString cleanStr = stripHtmlTags(str);
	
	// Skip output if result is empty or just whitespace
	if (cleanStr.trimmed().isEmpty() && url.isEmpty()) {
		return;
	}
	
	// Clean up meaningless URLs like "index.html?_" that provide no useful information
	if (!url.isEmpty() && (url == "index.html?_" || url.startsWith("index.html") || url == "_")) {
		// Only output if there's meaningful description
		if (cleanStr.trimmed().isEmpty() || cleanStr.length() < 3) {
			return;
		}
		url.clear(); // Don't show meaningless URL
	}
	
	// Try to extract login:password patterns from cleaned string or HTML
	// Pattern 1: login:pass@url (from href="http://login:pass@url" or visible text)
	QRegularExpression loginPassRegex1(R"(([^\s:@]+):([^\s@]+)@([^\s<>"']+))");
	QRegularExpressionMatch match1 = loginPassRegex1.match(cleanStr);
	if (!match1.hasMatch()) {
		// Also check in original HTML string (might be in href)
		match1 = loginPassRegex1.match(str);
	}
	
	// Pattern 2: url (login:pass) format
	QRegularExpression loginPassRegex2(R"(([^\s<>"']+)\s*\(([^\s:]+):([^\s)]+)\))");
	QRegularExpressionMatch match2 = loginPassRegex2.match(cleanStr);
	
	if (match1.hasMatch()) {
		QString login = match1.captured(1);
		QString pass = match1.captured(2);
		QString authUrl = match1.captured(3);
		
		// Output formatted
		out << ANSI_MAGENTA << ANSI_BOLD << "[AUTH]" << ANSI_RESET << " ";
		out << ANSI_BOLD << ANSI_GREEN << login << ANSI_RESET << ANSI_GRAY << ":" << ANSI_RESET 
		    << ANSI_BOLD << ANSI_YELLOW << pass << ANSI_RESET;
		out << ANSI_GRAY << "@" << ANSI_RESET;
		out << ANSI_CYAN << authUrl << ANSI_RESET;
		
		// Extract additional info (title, camera type, etc.)
		QString rest = cleanStr;
		rest.remove(QRegularExpression(QString("(%1:%2@%3)").arg(QRegularExpression::escape(login), 
		                                                          QRegularExpression::escape(pass),
		                                                          QRegularExpression::escape(authUrl))));
		rest.remove(login + ":" + pass + "@" + authUrl);
		
		// Clean up prefixes and formatting
		rest.remove(QRegularExpression("^\\s*\\[BA\\]:?\\s*"));
		rest.remove(QRegularExpression("^\\s*\\[WIC\\]:?\\s*"));
		rest.remove(QRegularExpression("^\\s*\\[WF\\]:?\\s*"));
		rest.remove(QRegularExpression("^\\s*\\[SVC\\]:?\\s*"));
		rest.remove(QRegularExpression("^\\s*\\[RTSP\\]:?\\s*"));
		rest.remove(QRegularExpression("^\\s*T:\\s*"));
		
		rest = rest.trimmed();
		if (!rest.isEmpty()) {
			out << ANSI_GRAY << " : " << ANSI_RESET << rest;
		}
	}
	else if (match2.hasMatch()) {
		QString authUrl = match2.captured(1);
		QString login = match2.captured(2);
		QString pass = match2.captured(3);
		
		// Output formatted
		out << ANSI_MAGENTA << ANSI_BOLD << "[AUTH]" << ANSI_RESET << " ";
		out << ANSI_CYAN << authUrl << ANSI_RESET;
		out << ANSI_GRAY << " (" << ANSI_RESET;
		out << ANSI_BOLD << ANSI_GREEN << login << ANSI_RESET << ANSI_GRAY << ":" << ANSI_RESET 
		    << ANSI_BOLD << ANSI_YELLOW << pass << ANSI_RESET;
		out << ANSI_GRAY << ")" << ANSI_RESET;
		
		// Extract additional info
		QString rest = cleanStr;
		rest.remove(QRegularExpression(QString("%1\\s*\\(%2:%3\\)").arg(QRegularExpression::escape(authUrl),
		                                                                  QRegularExpression::escape(login),
		                                                                  QRegularExpression::escape(pass))));
		
		// Clean up prefixes
		rest.remove(QRegularExpression("^\\s*\\[BA\\]:?\\s*"));
		rest.remove(QRegularExpression("^\\s*\\[WIC\\]:?\\s*"));
		rest.remove(QRegularExpression("^\\s*\\[WF\\]:?\\s*"));
		rest.remove(QRegularExpression("^\\s*\\[SVC\\]:?\\s*"));
		rest.remove(QRegularExpression("^\\s*\\[RTSP\\]:?\\s*"));
		rest.remove(QRegularExpression("^\\s*T:\\s*"));
		
		rest = rest.trimmed();
		if (!rest.isEmpty()) {
			out << ANSI_GRAY << " : " << ANSI_RESET << rest;
		}
	} else {
		// No login/password, just format URL and text
		if (url.length() > 0 && url != cleanStr) {
			// Check if cleanStr contains the URL (to avoid duplication)
			QString rest = cleanStr;
			if (rest.contains(url)) {
				rest.remove(url);
				// Also remove any protocol prefixes if URL was in href
				rest.remove(QRegularExpression("https?://"));
			}
			
			// Clean up prefixes
			rest.remove(QRegularExpression("^\\s*\\[BA\\]:?\\s*"));
			rest.remove(QRegularExpression("^\\s*\\[WIC\\]:?\\s*"));
			rest.remove(QRegularExpression("^\\s*\\[WF\\]:?\\s*"));
			rest.remove(QRegularExpression("^\\s*\\[SVC\\]:?\\s*"));
			rest.remove(QRegularExpression("^\\s*\\[RTSP\\]:?\\s*"));
			rest.remove(QRegularExpression("^\\s*T:\\s*"));
			
			rest = rest.trimmed();
			// Only show separator if there's meaningful content (more than 1 character, not just ":")
			if (!rest.isEmpty() && rest.length() > 1 && rest != ":") {
				// Limit description length for readability
				if (rest.length() > 150) {
					rest = rest.left(147) + "...";
				}
				out << ANSI_CYAN << url << ANSI_RESET;
				out << ANSI_GRAY << " : " << ANSI_RESET << rest;
			} else {
				// If no description, just show URL
				out << ANSI_CYAN << url << ANSI_RESET;
			}
		} else if (!cleanStr.trimmed().isEmpty()) {
			// Regular text, just strip HTML
			QString rest = cleanStr;
			rest.remove(QRegularExpression("^\\s*\\[BA\\]:?\\s*"));
			rest.remove(QRegularExpression("^\\s*\\[WIC\\]:?\\s*"));
			rest.remove(QRegularExpression("^\\s*\\[WF\\]:?\\s*"));
			rest.remove(QRegularExpression("^\\s*\\[SVC\\]:?\\s*"));
			rest.remove(QRegularExpression("^\\s*\\[RTSP\\]:?\\s*"));
			rest = rest.trimmed();
			if (!rest.isEmpty() && rest != ":") {
				// Limit description length for readability
				if (rest.length() > 150) {
					rest = rest.left(147) + "...";
				}
				out << rest;
			}
		} else if (!url.isEmpty()) {
			// Only URL, no description
			out << ANSI_CYAN << url << ANSI_RESET;
		}
	}
	
	out << Qt::endl;
}

void STh::doEmitionRedFoundData(QString str)
{
	QTextStream err(stderr);
	QString cleanStr = stripHtmlTags(str);
	
	// Check if this is a CURL error and enhance it
	QRegularExpression curlErrorRegex(R"(CURL error:?\s*\((\d+)\)\s*(.*))", QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch match = curlErrorRegex.match(cleanStr);
	
	if (match.hasMatch()) {
		int curlCode = match.captured(1).toInt();
		QString url = match.captured(2).trimmed();
		QString errorDesc = getCurlErrorString(curlCode);
		
		// Skip common non-critical errors (redirects are normal for scanning)
		if (curlCode == 47) return;
		
		// Format: [ERROR] CURL: <description> (<code>) <url>
		err << ANSI_RED << "[ERROR]" << ANSI_RESET << " CURL: " << ANSI_YELLOW << errorDesc 
		    << ANSI_RESET << " (" << curlCode << ")";
		if (!url.isEmpty()) {
			err << " " << ANSI_CYAN << url << ANSI_RESET;
		}
		err << Qt::endl;
		return;
	}
	
	// Check for "Curl error" (different capitalization, no code)
	QRegularExpression curlErrorRegex2(R"(Curl error:?\s*(.*))", QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch match2 = curlErrorRegex2.match(cleanStr);
	if (match2.hasMatch()) {
		QString url = match2.captured(1).trimmed();
		err << ANSI_RED << "[ERROR]" << ANSI_RESET << " CURL: " << ANSI_YELLOW << "Connection failed" 
		    << ANSI_RESET;
		if (!url.isEmpty()) {
			err << " " << ANSI_CYAN << url << ANSI_RESET;
		}
		err << Qt::endl;
		return;
	}
	
	// Regular error message
	err << ANSI_RED << "[ERROR]" << ANSI_RESET << " " << cleanStr << Qt::endl;
}

void STh::doEmitionGreenFoundData(QString str)
{
	QTextStream out(stdout);
	QString cleanStr = stripHtmlTags(str);
	out << ANSI_GREEN << "[OK]" << ANSI_RESET << " " << cleanStr << Qt::endl;
}

void STh::doEmitionFoundDataCustom(QString str, QString color)
{
	QTextStream out(stdout);
	QString cleanStr = stripHtmlTags(str);
	// Map hex colors to ANSI (simple mapping)
	if (color.toUpper() == "FF69B4" || color.toUpper().contains("PINK", Qt::CaseInsensitive)) {
		out << ANSI_MAGENTA << "→" << ANSI_RESET << " " << cleanStr << Qt::endl;
	} else {
		out << ANSI_GRAY << "→" << ANSI_RESET << " " << cleanStr << Qt::endl;
	}
}

void STh::doEmitionYellowFoundData(QString str)
{
	QTextStream out(stdout);
	QString cleanStr = stripHtmlTags(str);
	out << ANSI_YELLOW << "[WARN]" << ANSI_RESET << " " << cleanStr << Qt::endl;
}

void STh::doEmitionDebugFoundData(QString str)
{
	QTextStream out(stdout);
	QString cleanStr = stripHtmlTags(str);
	out << ANSI_GRAY << "[DEBUG]" << ANSI_RESET << " " << cleanStr << Qt::endl;
}

void STh::doEmitionKillSttThread()
{
	QTextStream out(stdout);
	out << ANSI_YELLOW << "[INFO] Stopping scan thread..." << ANSI_RESET << Qt::endl;
}

void STh::doEmitionDataSaved(bool status)
{
	QTextStream out(stdout);
	if (status) {
		out << ANSI_GREEN << "[OK] Data saved successfully" << ANSI_RESET << Qt::endl;
	}
}

void STh::doEmitionUpdateArc(unsigned long gTargets)
{
	QTextStream out(stdout);
	// Calculate percentage if we have total
	extern unsigned long long gTargetsNumber;
	
	// Throttle progress updates to avoid too frequent screen updates
	unsigned long processed = (gTargetsNumber > 0 && gTargets <= gTargetsNumber) ? 
		(gTargetsNumber - gTargets) : 0;
	
	// Update only every N processed items or if significant change
	unsigned long progressDiff = (processed > g_lastProgressUpdate) ? 
		(processed - g_lastProgressUpdate) : (g_lastProgressUpdate - processed);
	
	if (progressDiff < PROGRESS_UPDATE_INTERVAL && g_lastProgressUpdate > 0) {
		// Skip update if too soon
		return;
	}
	
	g_lastProgressUpdate = processed;
	
	QString progressStr;
	if (gTargetsNumber > 0 && gTargets <= gTargetsNumber) {
		double percent = ((double)processed / (double)gTargetsNumber) * 100.0;
		QString percentStr = QString::number(percent, 'f', 1);
		progressStr = QString("[%1/%2] %3% | Remaining: %4")
			.arg(processed)
			.arg(gTargetsNumber)
			.arg(percentStr)
			.arg(gTargets);
	} else {
		progressStr = QString("Remaining: %1").arg(gTargets);
	}
	
	// Clear line and output progress (use \r to overwrite same line)
	out << "\r" << ANSI_BOLD << ANSI_CYAN << progressStr << ANSI_RESET;
	out.flush();
}

void STh::doEmitionBlockButton(bool value)
{
	// В консольной версии не нужно блокировать кнопки
	Q_UNUSED(value);
}

int STh::baModelSize()
{
	return 0; // В консольной версии не поддерживается
}

void STh::setMode(short mode) {
	gMode = mode;
}
void STh::setTarget(QString target) {
	this->target = target;
}
void STh::setPorts(QString ports) {
	this->ports = ports;
}
void STh::run() 
{
	MainStarter ms;
	ms.start(this->target.toLocal8Bit().data(),
		this->ports.toLocal8Bit().data());
}
