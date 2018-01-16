using Google.Apis.Auth.OAuth2;
using Google.Apis.Sheets.v4;
using Google.Apis.Drive.v3;
using Google.Apis.Drive.v3.Data;
using Google.Apis.Sheets.v4.Data;
using Google.Apis.Services;
using Google.Apis.Util.Store;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.Ports;
using System.IO;
using Newtonsoft.Json;
using System.Threading;

using Data = Google.Apis.Sheets.v4.Data;

namespace verticalFarmConsole
{
    class Program
    {
        // If modifying these scopes, delete your previously saved sheetCredentials
        // at ~/.sheetCredentials/sheets.googleapis.com-dotnet-quickstart.json
        static string[] Scopes = { SheetsService.Scope.Spreadsheets, DriveService.Scope.Drive };
        static string ApplicationName = "Google Sheets/Drive API .NET Quickstart";

        static void Main(string[] args)
        {
            /*
             * Set up API services
             */

            UserCredential credential;

            using (var stream =
                new FileStream("client_secret.json", FileMode.Open, FileAccess.Read))
            {
                string credPath = System.Environment.GetFolderPath(
                    System.Environment.SpecialFolder.Personal);
                credPath = Path.Combine(credPath, ".credentials/drive-dotnet-quickstart.json");

                credential = GoogleWebAuthorizationBroker.AuthorizeAsync(
                    GoogleClientSecrets.Load(stream).Secrets,
                    Scopes,
                    "user",
                    CancellationToken.None,
                    new FileDataStore(credPath, true)).Result;
                Console.WriteLine("Google API credential file saved to: " + credPath);
            }

            // Create Google Sheets API service.
            var sheetsService = new SheetsService(new BaseClientService.Initializer()
            {
                HttpClientInitializer = credential,
                ApplicationName = ApplicationName,
            });

            // Create Drive API service.
            var driveService = new DriveService(new BaseClientService.Initializer()
            {
                HttpClientInitializer = credential,
                ApplicationName = ApplicationName,
            });

            /*
             * USB Port Config
             */

            var port = new SerialPort();

            //Get user input
            Console.Write("Enter port number: ");
            String portNum = Console.ReadLine().Substring(0, 1);

            try
            {
                port.PortName = "COM" + portNum;
                port.BaudRate = 9600;
                port.ReadTimeout = 500;
                port.WriteTimeout = 500;
                port.Open();
                port.DiscardInBuffer();

                Console.WriteLine("Opened serial communication on COM" + portNum);
            }
            catch (Exception)
            {
                Console.WriteLine("ERROR: Failed to open COM" + portNum);
            }

            var propertiesFileLocation = @"C:\Users\danie\Desktop\VFproperties.txt";
            String machineSN = "";
            String spreadsheetID = "";
            String folderID = "1YDdzdk8XuOHjplu_OcLPgwEl9Gnjk9LX";
            try
            {
                System.IO.StreamReader propertiesFile = new System.IO.StreamReader(propertiesFileLocation);
                String propertiesLine1 = propertiesFile.ReadLine(); //Machine SN
                String propertiesLine2 = propertiesFile.ReadLine(); //Current Spreadsheet ID
                String propertiesLine3 = propertiesFile.ReadLine(); //Folder ID
                propertiesFile.Close();

                machineSN = propertiesLine1.Substring(propertiesLine1.IndexOf(':') + 2);
                spreadsheetID = propertiesLine2.Substring(propertiesLine2.IndexOf(':') + 2);
                folderID = propertiesLine3.Substring(propertiesLine2.IndexOf(':') + 2);

                Console.WriteLine("Machine SN: " + machineSN);
                Console.WriteLine("Spreadsheet ID: " + spreadsheetID);
                Console.WriteLine("Folder ID: " + folderID);
            }
            catch (Exception)
            {
                Console.WriteLine("ERROR: Failed to locate or read VFproperties.txt, ensure this is the correct directory: " + propertiesFileLocation);
            }

            while (true)
            {
                if (port.IsOpen)
                {
                    if (port.BytesToRead > 0)
                    {
                        String SN = "";
                        String tempA = "";
                        String tempW = "";
                        String humidity = "";
                        String RSC = "";
                        String LSC = "";
                        String watered = "";
                        String errorCode = "";
                        String dateTime = DateTime.Now.ToString();

                        String input = port.ReadLine();
                        if (input.Contains("Clear") || input.Contains("Start"))
                        {
                            Console.WriteLine(dateTime + " " + input);
                        }
                        else if (input.Contains("ERROR"))
                        {
                            errorCode = input.Substring(input.IndexOf(':') + 2);

                            /*
                            * Add data to spreadsheet
                            */

                            String range = "Sheet1!A:I";
                            // How the input data should be interpreted.
                            SpreadsheetsResource.ValuesResource.AppendRequest.ValueInputOptionEnum valueInputOption = SpreadsheetsResource.ValuesResource.AppendRequest.ValueInputOptionEnum.USERENTERED;

                            // How the input data should be inserted.
                            SpreadsheetsResource.ValuesResource.AppendRequest.InsertDataOptionEnum insertDataOption = SpreadsheetsResource.ValuesResource.AppendRequest.InsertDataOptionEnum.INSERTROWS;

                            Data.ValueRange requestBody = new Data.ValueRange();
                            requestBody.Range = range;
                            requestBody.MajorDimension = "ROWS";
                            var oblist = new List<object>() { dateTime, SN, tempA, tempW, humidity, RSC, LSC, watered, errorCode };
                            requestBody.Values = new List<IList<object>> { oblist };

                            SpreadsheetsResource.ValuesResource.AppendRequest appendRequest = sheetsService.Spreadsheets.Values.Append(requestBody, spreadsheetID, range);
                            appendRequest.ValueInputOption = valueInputOption;
                            appendRequest.InsertDataOption = insertDataOption;
                            Data.AppendValuesResponse updateResponse = appendRequest.Execute();

                            String rangeReturn = JsonConvert.SerializeObject(updateResponse.Updates.UpdatedRange);
                            rangeReturn = rangeReturn.Substring(rangeReturn.IndexOf("!A") + 2);
                            rangeReturn = rangeReturn.Substring(0, rangeReturn.IndexOf(':'));
                            int i = Convert.ToInt32(rangeReturn);   //row it was put into

                            /*
                            * Print data to console
                            */

                            Console.WriteLine(dateTime + ", SN: " + SN + ", Temp A (c): " + tempA + ", Temp W (c): " + tempW + ", Humidity (%): " + humidity + ", RSC: " + RSC + ", LSC: " + LSC + ", Watered: " + watered + ", Error: " + errorCode);

                            if (i >= 10000)
                            {
                                /*
                                 * Create new spreadsheet
                                 */

                                Data.Spreadsheet requestBody1 = new Data.Spreadsheet();
                                requestBody1.Properties = new SpreadsheetProperties();
                                requestBody1.Properties.Title = dateTime;
                                SpreadsheetsResource.CreateRequest request = sheetsService.Spreadsheets.Create(requestBody1);
                                Data.Spreadsheet response = request.Execute();
                                spreadsheetID = JsonConvert.SerializeObject(response.SpreadsheetId);
                                spreadsheetID = spreadsheetID.Substring(1, spreadsheetID.Length - 2);
                                Console.WriteLine(spreadsheetID);

                                /*
                                 * Move spreadsheet to correct folder
                                 */
                                
                                // Retrieve the existing parents to remove
                                var getRequest = driveService.Files.Get(spreadsheetID);
                                getRequest.Fields = "parents";
                                var file = getRequest.Execute();
                                var previousParents = String.Join(",", file.Parents);
                                // Move the file to the new folder
                                var updateRequest = driveService.Files.Update(new Google.Apis.Drive.v3.Data.File(), spreadsheetID);
                                updateRequest.Fields = "id, parents";
                                updateRequest.AddParents = folderID;
                                updateRequest.RemoveParents = previousParents;
                                file = updateRequest.Execute();

                                /*
                                 * Add headers to spreadsheet
                                 */
                                
                                oblist = new List<object>() { "Date/Time", "Serial Number", "Air Temp (c)", "Water Temp (c)", "Humidity (%)", "RSC", "LSC", "Watered", "Error" };
                                requestBody.Values = new List<IList<object>> { oblist };

                                appendRequest = sheetsService.Spreadsheets.Values.Append(requestBody, spreadsheetID, range);
                                updateResponse = appendRequest.Execute();

                                /*
                                 * Update VFproperties.txt with new ID's
                                 */
                                
                                String[] lines = { "Machine Serial Number: " + machineSN, "Current Spreadsheet ID: " + spreadsheetID, "Google Drive Folder ID: " + folderID };
                                System.IO.File.WriteAllLines(propertiesFileLocation, lines);
                            }
                        }
                        else
                        {
                            SN = input.Substring(0, input.IndexOf(','));
                            input = input.Substring(input.IndexOf(',') + 1);
                            tempA = input.Substring(0, input.IndexOf(','));
                            input = input.Substring(input.IndexOf(',') + 1);
                            tempW = input.Substring(0, input.IndexOf(','));
                            input = input.Substring(input.IndexOf(',') + 1);
                            humidity = input.Substring(0, input.IndexOf(','));
                            input = input.Substring(input.IndexOf(',') + 1);
                            RSC = input.Substring(0, input.IndexOf(','));
                            input = input.Substring(input.IndexOf(',') + 1);
                            LSC = input.Substring(0, input.IndexOf(','));
                            input = input.Substring(input.IndexOf(',') + 1);
                            watered = input.Substring(0, 1);

                            /*
                            * Add data to spreadsheet
                            */

                            String range = "Sheet1!A:I";
                            // How the input data should be interpreted.
                            SpreadsheetsResource.ValuesResource.AppendRequest.ValueInputOptionEnum valueInputOption = SpreadsheetsResource.ValuesResource.AppendRequest.ValueInputOptionEnum.USERENTERED;

                            // How the input data should be inserted.
                            SpreadsheetsResource.ValuesResource.AppendRequest.InsertDataOptionEnum insertDataOption = SpreadsheetsResource.ValuesResource.AppendRequest.InsertDataOptionEnum.INSERTROWS;

                            Data.ValueRange requestBody = new Data.ValueRange();
                            requestBody.Range = range;
                            requestBody.MajorDimension = "ROWS";
                            var oblist = new List<object>() { dateTime, SN, tempA, tempW, humidity, RSC, LSC, watered, errorCode };
                            requestBody.Values = new List<IList<object>> { oblist };

                            SpreadsheetsResource.ValuesResource.AppendRequest appendRequest = sheetsService.Spreadsheets.Values.Append(requestBody, spreadsheetID, range);
                            appendRequest.ValueInputOption = valueInputOption;
                            appendRequest.InsertDataOption = insertDataOption;
                            Data.AppendValuesResponse updateResponse = appendRequest.Execute();

                            String rangeReturn = JsonConvert.SerializeObject(updateResponse.Updates.UpdatedRange);
                            rangeReturn = rangeReturn.Substring(rangeReturn.IndexOf("!A") + 2);
                            rangeReturn = rangeReturn.Substring(0, rangeReturn.IndexOf(':'));
                            int i = Convert.ToInt32(rangeReturn);   //row it was put into

                            /*
                            * Print data to console
                            */

                            Console.WriteLine(dateTime + ", SN: " + SN + ", Temp A (c): " + tempA + ", Temp W (c): " + tempW + ", Humidity (%): " + humidity + ", RSC: " + RSC + ", LSC: " + LSC + ", Watered: " + watered + ", Error: " + errorCode);

                            if (i >= 10000)
                            {
                                /*
                                 * Create new spreadsheet
                                 */

                                Data.Spreadsheet requestBody1 = new Data.Spreadsheet();
                                requestBody1.Properties = new SpreadsheetProperties();
                                requestBody1.Properties.Title = dateTime;
                                SpreadsheetsResource.CreateRequest request = sheetsService.Spreadsheets.Create(requestBody1);
                                Data.Spreadsheet response = request.Execute();
                                spreadsheetID = JsonConvert.SerializeObject(response.SpreadsheetId);
                                spreadsheetID = spreadsheetID.Substring(1, spreadsheetID.Length - 2);
                                Console.WriteLine(spreadsheetID);

                                /*
                                 * Move spreadsheet to correct folder
                                 */

                                // Retrieve the existing parents to remove
                                var getRequest = driveService.Files.Get(spreadsheetID);
                                getRequest.Fields = "parents";
                                var file = getRequest.Execute();
                                var previousParents = String.Join(",", file.Parents);
                                // Move the file to the new folder
                                var updateRequest = driveService.Files.Update(new Google.Apis.Drive.v3.Data.File(), spreadsheetID);
                                updateRequest.Fields = "id, parents";
                                updateRequest.AddParents = folderID;
                                updateRequest.RemoveParents = previousParents;
                                file = updateRequest.Execute();

                                /*
                                 * Add headers to spreadsheet
                                 */

                                oblist = new List<object>() { "Date/Time", "Serial Number", "Air Temp (c)", "Water Temp (c)", "Humidity (%)", "RSC", "LSC", "Watered", "Error" };
                                requestBody.Values = new List<IList<object>> { oblist };

                                appendRequest = sheetsService.Spreadsheets.Values.Append(requestBody, spreadsheetID, range);
                                updateResponse = appendRequest.Execute();

                                /*
                                 * Update VFproperties.txt with new ID's
                                 */

                                String[] lines = { "Machine Serial Number: " + machineSN, "Current Spreadsheet ID: " + spreadsheetID, "Google Drive Folder ID: " + folderID };
                                System.IO.File.WriteAllLines(propertiesFileLocation, lines);
                            }
                        }
                    }
                }
            }

            port.Close();
        }
    }
}
