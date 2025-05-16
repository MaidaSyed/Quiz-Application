#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

// Function to print styled Heading
void printHeading(const char *heading) {
    printf("\n=========================================\n");
    printf("           %s\n", heading);
    printf("=========================================\n");
}

// Structure to store user data
struct User {
    char username[30];
    char password[30];
    char quizSubject[30];
    int marks;
};

// Subjects array
char subjects[5][40] = {
    "Programming Fundamentals",
    "Functional English",
    "Applied Physics",
    "Calculus",
    "ICP",
};

// Structure to store question data for each quiz
struct Question {
    char question[200];
    char options[4][100];
    int correctAnswer;
};

// Function to save user data to file
void saveUser(struct User user) {
    FILE *file = fopen("users.txt", "a+");

    if (file == NULL) {
        printf("Could not open file!\n");
        return;
    }

    fwrite(&user, sizeof(struct User), 1, file);
    fclose(file);
    printf("User registered successfully!\n");
}

// Function to check if a username already exists
int isUserExists(char username[]) {
    FILE *file = fopen("users.txt", "r");

    if (file == NULL) {
        return 0;  // No users registered yet
    }

    struct User user;
    while (fread(&user, sizeof(struct User), 1, file)) {
        if (strcmp(user.username, username) == 0) {
            fclose(file);
            return 1;  // User exists
        }
    }

    fclose(file);
    return 0;  // User does not exist
}

// Function to validate user login
int userLogin(char username[], char password[]) {
    FILE *file = fopen("users.txt", "r");

    if (file == NULL) {
        printf("No users registered yet!\n");
        return 0;
    }

    struct User user;
    while (fread(&user, sizeof(struct User), 1, file)) {
        if (strcmp(user.username, username) == 0 && strcmp(user.password, password) == 0) {
            fclose(file);
            return 1;  // Login successful
        }
    }

    fclose(file);
    return 0;  // Login failed
}

// Function to show user profile
void trimNewline(char *str) {
    // Remove newline character if present
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}
void showUserProfile(char username[]) {
    FILE *file;
    char line[100];
    
    trimNewline(username);

    // List of subjects
    const char *subjects[] = {
        "Programming Fundamentals",
        "Functional English",
        "Applied Physics",
        "Calculus",
        "ICP"
    };
    printHeading("User Profile");
    printf("Username: %s\n", username);

    // Loop over each subject and try to find the marks for the user
    for (int i = 0; i < 5; i++) {
        char filename[50];
        sprintf(filename, "%s_scores.txt", subjects[i]);
        
        // Open the file for the current subject
        file = fopen(filename, "r");
        if (file == NULL) {
            printf("Could not open file for subject: %s\n", subjects[i]);
            continue; 
        }

        int found = 0;
        // Read through the file line by line and search for the username
        while (fgets(line, sizeof(line), file)) {
            char fileUsername[50];
            int marks;
            sscanf(line, "Username: %49[^,], Marks: %d", fileUsername, &marks);

            trimNewline(fileUsername);

            // Compare the username in the file with the input username
            if (strcmp(fileUsername, username) == 0) {
                // If the username matches, print the marks
                printf("%s Marks: %d\n", subjects[i], marks);
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("%s Marks: This subject quiz has not been attempted.\n", subjects[i]);
        }

        fclose(file);
    }
}

// Function to load questions for a subject from file
void loadQuestions(char subject[], struct Question questions[15]) {
    char filename[50];
    snprintf(filename, sizeof(filename), "%s_questions.txt", subject); // File name based on subject

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not load questions for %s.\n", subject);
        return;
    }

    // Load 15 questions into the array
    for (int i = 0; i < 15; i++) {
        fgets(questions[i].question, sizeof(questions[i].question), file);
        for (int j = 0; j < 4; j++) {
            fgets(questions[i].options[j], sizeof(questions[i].options[j]), file);
        }
        fscanf(file, "%d", &questions[i].correctAnswer);
        fgetc(file); // Consume the newline character after correctAnswer
    }

    fclose(file);
}

// Function to save user marks in the subject's file
void saveUserMarks(char username[], char subject[], int marks) {
    // Determine the filename based on the subject
    char filename[50];
    snprintf(filename, sizeof(filename), "%s_scores.txt", subject);

    FILE *file = fopen(filename, "r+"); 
    if (file == NULL) {
        printf("Error opening file to save marks.\n");
        return;
    }

    char line[100];
    int userFound = 0;
    char content[1000] = "";
    
    // Read the file and search for the user
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, username)) {  // Check if username exists in the line
            userFound = 1;
            // Update the line with the new marks for this user
            sprintf(line, "Username: %s, Marks: %d\n", username, marks);
        }
        strcat(content, line);
    }

    fclose(file);

    // Reopen the file in write mode to overwrite it with the updated content
    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file to write updated marks.\n");
        return;
    }

    // Write the updated content to the file
    fprintf(file, "%s", content);

    if (userFound) {
        printf("Marks updated for user %s.\n", username);
    } else {
        // If the user was not found, append the new entry at the end
        fprintf(file, "Username: %s, Marks: %d\n", username, marks);
        printf("New user marks added for %s.\n", username);
    }

    fclose(file);
}

// Start quiz with a timer
int remainingTime = 20;
int timerRunning = 1;   
int terminalWidth = 80; 

// Function for the timer thread
void* showTimer(void* arg) {
    while (remainingTime > 0 && timerRunning) {
        // Move cursor to top-right corner
        printf("\033[s");                              
        printf("\033[%d;%dH", 1, terminalWidth - 20); 
        printf("Time left: %3d seconds", remainingTime);
        printf("\033[u");                              
        fflush(stdout);
        sleep(1);
        remainingTime--;
    }

    // Stop the quiz if time is up
    if (remainingTime == 0) {
        timerRunning = 0;
    }
    return NULL;
}

// Function to start the quiz
void startQuiz(char subject[], char username[]) {
    struct Question questions[50];
    int questionCount = 0;

    remainingTime = 20;
    timerRunning = 1;

    loadQuestions(subject, questions);

    while (questions[questionCount].question[0] != '\0') {
        questionCount++;
    }

    if (questionCount < 15) {
        printf("Not enough questions available for a quiz in %s.\n", subject);
        return;
    }

    int selectedQuestions[15];
    int selectedCount = 0;
    srand(time(0));

    // Select 15 unique random questions
    while (selectedCount < 15) {
        int randomIndex = rand() % questionCount;
        int alreadySelected = 0;

        for (int i = 0; i < selectedCount; i++) {
            if (selectedQuestions[i] == randomIndex) {
                alreadySelected = 1;
                break;
            }
        }

        if (!alreadySelected) {
            selectedQuestions[selectedCount++] = randomIndex;
        }
    }

    int marks = 0, answer;
    FILE *wrongAnswersFile = fopen("wrong_answers.tmp", "w");

    if (!wrongAnswersFile) {
        printf("Error: Could not create temporary file for wrong answers.\n");
        return;
    }

    printf("\nStarting the Quiz for %s. You have 2.5 minutes!\n\n", subject);

    // Start the timer thread
    pthread_t timerThread;
    pthread_create(&timerThread, NULL, showTimer, NULL);

    for (int i = 0; i < 15; i++) {
        // Check if time is up before displaying the question
        if (!timerRunning) {
            printf("\n\nTime's up! Ending the quiz...\n");
            break;
        }

        // Display the question
        int qIndex = selectedQuestions[i];
        printf("\nQuestion %d: %s\n", i + 1, questions[qIndex].question);
        for (int j = 0; j < 4; j++) {
            printf("%d. %s\n", j + 1, questions[qIndex].options[j]);
        }

        printf("Enter your answer (1-4): ");

        while (timerRunning) {
            if (scanf("%d", &answer) == 1 && answer >= 1 && answer <= 4) {
                break;
            } else if (!timerRunning) {
                break;
            } else {
                printf("Invalid input! Enter a number between 1 and 4: ");
                while (getchar() != '\n'); // Clear input buffer
            }
        }

        if (!timerRunning) {
            printf("\n\nTime's up! Ending the quiz...\n");
            break;
        }

        if (answer == questions[qIndex].correctAnswer) {
            marks++;
        } else {
            fprintf(wrongAnswersFile, "Q%d: %s\nCorrect Answer: %d. %s\n\n", 
                    i + 1, 
                    questions[qIndex].question, 
                    questions[qIndex].correctAnswer, 
                    questions[qIndex].options[questions[qIndex].correctAnswer - 1]);
        }

        if (i == 14) {
            timerRunning = 0;
        }
    }

    // Ensure timer thread is joined properly
    pthread_join(timerThread, NULL);

    // Display final marks and wrong answers
    printf("\nQuiz Complete! You scored %d marks in the %s quiz.\n", marks, subject);

    fclose(wrongAnswersFile);

    // Display wrong answers
    printf("\nReview of Wrong Answers:\n");
    wrongAnswersFile = fopen("wrong_answers.tmp", "r");
    if (wrongAnswersFile) {
        char line[256];
        while (fgets(line, sizeof(line), wrongAnswersFile)) {
            printf("%s", line);
        }
        fclose(wrongAnswersFile);
        remove("wrong_answers.tmp");
    } else {
        printf("Error: Could not read temporary file for wrong answers.\n");
    }

    saveUserMarks(username, subject, marks);
}

// function to add questions
void addQuestions() {
    printf("\n--- Subjects ---\n");
    for (int i = 0; i < 5; i++) {
        printf("%d. %s\n", i + 1, subjects[i]);
    }

    int subjectChoice;
    printf("Enter the number corresponding to the subject: ");
    scanf("%d", &subjectChoice);
    getchar();

    if (subjectChoice < 1 || subjectChoice > 5) {
        printf("Invalid choice. Exiting.\n");
        return;
    }

    char *subject = subjects[subjectChoice - 1];
    char filename[50];
    snprintf(filename, sizeof(filename), "%s_questions.txt", subject);

    FILE *file = fopen(filename, "a+");
    if (file == NULL) {
        printf("Error: Could not open file for %s.\n", subject);
        return;
    }

    int addMore = 1;
    while (addMore) {
        struct Question newQuestion;
        printf("\nEnter the question text: ");
        fgets(newQuestion.question, sizeof(newQuestion.question), stdin);
        newQuestion.question[strcspn(newQuestion.question, "\n")] = '\0';

        for (int i = 0; i < 4; i++) {
            printf("Enter Option %d: ", i + 1);
            fgets(newQuestion.options[i], sizeof(newQuestion.options[i]), stdin);
            newQuestion.options[i][strcspn(newQuestion.options[i], "\n")] = '\0'; 
        }

        while(newQuestion.correctAnswer > 4 || newQuestion.correctAnswer < 0) {
            printf("Enter the correct answer (1-4): ");
            scanf("%d", &newQuestion.correctAnswer);
            getchar();	
		}

        // Append the question to the file
        fprintf(file, "%s\n", newQuestion.question);
        for (int i = 0; i < 4; i++) {
            fprintf(file, "%s\n", newQuestion.options[i]);
        }
        fprintf(file, "%d\n", newQuestion.correctAnswer);

        printf("Question added successfully!\n");

        printf("Do you want to add another question? (1 for Yes, 0 for No): ");
        scanf("%d", &addMore);
        getchar();
    }

    fclose(file);
}

// Function to display marks for all students for a given subject
void viewMarksForSubject(char subject[]) {
    char filename[50];
    snprintf(filename, sizeof(filename), "%s_scores.txt", subject); // Construct filename based on subject

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file to view marks for subject %s.\n", subject);
        return;
    }

    char line[100];
    printf("\n--- Marks for %s ---\n", subject);

    // Read and display each student's marks using fgets
    while (fgets(line, sizeof(line), file)) {
        printf(" %s", line); 
        // Try parsing with sscanf after reading the line
//        char username[30];
//        int marks;
//        if (sscanf(line, "Username: %s, Marks: %d", username, &marks) == 2) {
//            printf("Student: %s, Marks: %d\n", username, marks);
//        } 
    }
    fclose(file);
}

// FUNCTION TO EDIT QUESTIONS
void editQuestions() {
    printHeading("Subjects");
    for (int i = 0; i < 5; i++) { // Adjusted for 5 subjects
        printf("%d. %s\n", i + 1, subjects[i]);
    }

    int subjectChoice;
    printf("Enter the number corresponding to the subject: ");
    scanf("%d", &subjectChoice);
    getchar(); // Clear the input buffer

    if (subjectChoice < 1 || subjectChoice > 5) { 
        printf("Invalid choice. Exiting.\n");
        return;
    }

    char *subject = subjects[subjectChoice - 1];
    char filename[50];
    snprintf(filename, sizeof(filename), "%s_questions.txt", subject);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file for %s.\n", subject);
        return;
    }

    struct Question questions[50];
    int questionCount = 0;

    // Read questions from the text file
    while (fgets(questions[questionCount].question, sizeof(questions[questionCount].question), file)) {
        questions[questionCount].question[strcspn(questions[questionCount].question, "\n")] = '\0'; 
        for (int i = 0; i < 4; i++) {  // Adjusted to 4 options (not 15)
            fgets(questions[questionCount].options[i], sizeof(questions[questionCount].options[i]), file);
            questions[questionCount].options[i][strcspn(questions[questionCount].options[i], "\n")] = '\0';
        }
        fscanf(file, "%d\n", &questions[questionCount].correctAnswer); // Read correct answer
        questionCount++;
    }
    fclose(file);

    if (questionCount == 0) {
        printf("No questions found for %s.\n", subject);
        return;
    }

    printf("\n--- Questions for %s ---\n", subject);
    for (int i = 0; i < questionCount; i++) {
        printf("%d. %s\n", i + 1, questions[i].question);
    }

    int questionNum;
    printf("Enter the number of the question you want to edit (1-%d): ", questionCount);
    scanf("%d", &questionNum);
    getchar(); // Clear the input buffer

    if (questionNum < 1 || questionNum > questionCount) {
        printf("Invalid question number!\n");
        return;
    }

    struct Question *selectedQuestion = &questions[questionNum - 1];
    printf("\n----Editing Question %d----\n", questionNum);

    printf("Current Question: %s\n", selectedQuestion->question);
    printf("Enter new question text: ");
    fgets(selectedQuestion->question, sizeof(selectedQuestion->question), stdin);
    selectedQuestion->question[strcspn(selectedQuestion->question, "\n")] = '\0';

    for (int i = 0; i < 4; i++) {  // 4 options remain
        printf("Current Option %d: %s\n", i + 1, selectedQuestion->options[i]);
        printf("Enter new text for Option %d: ", i + 1);
        fgets(selectedQuestion->options[i], sizeof(selectedQuestion->options[i]), stdin);
        selectedQuestion->options[i][strcspn(selectedQuestion->options[i], "\n")] = '\0';
    }
    
    int ans = 0;
    printf("Current Correct Answer: %d\n", selectedQuestion->correctAnswer);
    while(ans > 4 || ans < 1){
    	printf("Enter new correct answer (1-4): ");
    	scanf("%d", &ans);
	}
    selectedQuestion->correctAnswer = ans;
    // Save the updated questions back to the file
    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not open file for saving.\n");
        return;
    }

    for (int i = 0; i < questionCount; i++) {
        fprintf(file, "%s\n", questions[i].question);
        for (int j = 0; j < 4; j++) {
            fprintf(file, "%s\n", questions[i].options[j]);
        }
        fprintf(file, "%d\n", questions[i].correctAnswer);
    }

    fclose(file);

    printHeading("Question updated successfully!");
}

// Function to delete questions
void deleteQuestions() {
    printf("\n--- Subjects ---\n");
    for (int i = 0; i < 5; i++) {
        printf("%d. %s\n", i + 1, subjects[i]);
    }

    int subjectChoice;
    printf("Enter the number corresponding to the subject: ");
    scanf("%d", &subjectChoice);
    getchar();

    if (subjectChoice < 1 || subjectChoice > 5) {
        printf("Invalid choice. Exiting.\n");
        return;
    }

    char *subject = subjects[subjectChoice - 1];
    char filename[50];
    snprintf(filename, sizeof(filename), "%s_questions.txt", subject);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file for %s.\n", subject);
        return;
    }

    struct Question questions[50];
    int questionCount = 0;

    // Read questions from the text file
    while (fgets(questions[questionCount].question, sizeof(questions[questionCount].question), file)) {
        questions[questionCount].question[strcspn(questions[questionCount].question, "\n")] = '\0'; // Remove newline
        for (int i = 0; i < 4; i++) {
            fgets(questions[questionCount].options[i], sizeof(questions[questionCount].options[i]), file);
            questions[questionCount].options[i][strcspn(questions[questionCount].options[i], "\n")] = '\0'; // Remove newline
        }
        fscanf(file, "%d\n", &questions[questionCount].correctAnswer);
        questionCount++;
    }
    fclose(file);

    if (questionCount == 0) {
        printf("No questions found for %s.\n", subject);
        return;
    }

    printf("\n--- Questions for %s ---\n", subject);
    for (int i = 0; i < questionCount; i++) {
        printf("%d. %s\n", i + 1, questions[i].question);
    }

    int questionNum;
    printf("Enter the number of the question you want to delete (1-%d): ", questionCount);
    scanf("%d", &questionNum);
    getchar();

    if (questionNum < 1 || questionNum > questionCount) {
        printf("Invalid question number!\n");
        return;
    }

    // Remove the selected question by shifting others up
    for (int i = questionNum - 1; i < questionCount - 1; i++) {
        questions[i] = questions[i + 1];
    }
    questionCount--;

    // Rewrite the updated questions back to the file
    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not open file for saving.\n");
        return;
    }

    for (int i = 0; i < questionCount; i++) {
        fprintf(file, "%s\n", questions[i].question);
        for (int j = 0; j < 4; j++) {
            fprintf(file, "%s\n", questions[i].options[j]);
        }
        fprintf(file, "%d\n", questions[i].correctAnswer);
    }

    fclose(file);

    printf("Question deleted successfully!\n");
}

// Function to manage quiz questions (Admin only)
void manageQuiz() {
	int choice;
	char stdName[20];
	while(choice != 6) {
	printHeading("Admin Panel");
    printf("1. Edit Questions\n");
    printf("2. Add Questions\n");
    printf("3. Delete Questions\n");
    printf("4. View Marks for Subject\n");
    printf("5. Search Student\n");
    printf("6. Exit\n");
    printf("Enter your choice: ");
    
    scanf("%d", &choice);
    getchar();

    switch (choice) {
        case 1:
            editQuestions();
            break;
        case 2:
        	addQuestions();
        	break;
        case 3:
        	deleteQuestions();
        	break;
        case 4:
        	printHeading("Subjects");
            for (int i = 0; i < 5; i++) {
                printf("%d. %s\n", i + 1, subjects[i]);
            }
            int subjectChoice;
            printf("Enter the number corresponding to the subject: ");
            scanf("%d", &subjectChoice);
            getchar(); 

            if (subjectChoice >= 1 && subjectChoice <= 5) {
                viewMarksForSubject(subjects[subjectChoice - 1]);
            } else {
                printf("Invalid choice. Exiting.\n");
            }
            break;
        case 5:
        	printf("Enter Student Name: ");
        	scanf(" %s", stdName);
        	strlwr(stdName);
        	showUserProfile(stdName);
        	break;
        case 6:
        	printHeading("Exiting quiz management...\n");
            break;
        default:
            printf("Invalid choice! Try again.\n");
    }
}
}

// Main function
int main() {
    int choice;
    struct User newUser;
    char username[30], password[30];
    int loggedIn = 0;  // Track if a user is logged in
    system("color 0B");
    while (1) {
        if (!loggedIn) {
            // Show authentication panel only if no user is logged in
            printHeading("Authentication Panel");
            printf("1. Register\n");
            printf("2. User Login\n");
            printf("3. Admin Login\n");
            printf("4. Exit\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);
            getchar();

            switch (choice) {
                case 1:
                    // Register new user
                    printf("Enter username: ");
                    fgets(newUser.username, sizeof(newUser.username), stdin);
                    newUser.username[strcspn(newUser.username, "\n")] = '\0'; 

                    if (isUserExists(newUser.username)) {
                        printf("User already exists! Please login instead.\n");
                    } else {
                        printf("Enter password: ");
                        fgets(newUser.password, sizeof(newUser.password), stdin);
                        newUser.password[strcspn(newUser.password, "\n")] = '\0'; 
                        strcpy(newUser.quizSubject, "N/A");  // Placeholder for quiz subject
                        newUser.marks = 0;  // Placeholder for marks

                        saveUser(newUser);
                        loggedIn = 1;
                        strcpy(username, newUser.username);
                    }
                    break;

                case 2:
                    // User Login
                    printf("Enter username: ");
                    fgets(username, sizeof(username), stdin);
                    username[strcspn(username, "\n")] = '\0';  
                    printf("Enter password: ");
                    fgets(password, sizeof(password), stdin);
                    password[strcspn(password, "\n")] = '\0'; 

                    if (userLogin(username, password)) {
                        printf("User login successful!\n");
                        loggedIn = 1;
                    } else {
                        printf("Invalid username or password!\n");
                    }
                    break;

                case 3:
                    // Admin Login
                    printf("Enter admin username: ");
                    fgets(username, sizeof(username), stdin);
                    username[strcspn(username, "\n")] = '\0';  
                    printf("Enter admin password: ");
                    fgets(password, sizeof(password), stdin);
                    password[strcspn(password, "\n")] = '\0';

                    if (strcmp(username, "ramsha_iqbal") == 0 && strcmp(password, "PF_Project") == 0) {
                        printHeading("Admin login successful");
                        manageQuiz();  // Admin manages the quiz questions
                    } else {
                        printf("Invalid admin credentials!\n");
                    }
                    break;

                case 4:
                	printHeading("Quiz-Application By Team'C'");
                    return 0;

                default:
                    printf("Invalid choice! Please try again.\n");
            }
        } else {
            // Show user menu after login
            printHeading("User Menu");
            printf("1. View Profile\n");
            printf("2. Start a Quiz\n");
            printf("3. Log Out\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    showUserProfile(username);
                    break;

                case 2:
                    printf("Choose a subject to take the quiz:\n");
                    for (int i = 0; i < 5; i++) {
                        printf("%d. %s\n", i + 1, subjects[i]);
                    }
                    int subjectChoice;
                    printf("Enter your choice: ");
                    scanf("%d", &subjectChoice);
                    getchar(); 

                    if (subjectChoice >= 1 && subjectChoice <= 5) {
                        startQuiz(subjects[subjectChoice - 1], username);
                    } else {
                        printf("Invalid subject choice!\n");
                    }
                    break;

                case 3:
                    loggedIn = 0;
                    printf("You have logged out!\n");
                    break;

                default:
                    printf("Invalid choice! Please try again.\n");
            }
        }
    }

    return 0;
}
