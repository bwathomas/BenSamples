"""
What is this?

This is a simple, interactive data processing and visualization program I wrote. The goal of this program is
to disaggregate gender bias in reviews on ratemyprofessors. The user inputs a search word, and the window presents
six bar graphs, showing (to scale) the proportion of times that worst was used in low, middle, and high reviews
by gender.

Holistically, the program is composed of three parts:
biasbarsgui, which is a simple program (~20 lines) to generate the window and accept search terms. This was provided by the
professor.
biasbarsdata, which does the data processing.
biasbars, which displays the processed data on the window.
"""

KEY_WOMEN = "W"
KEY_MEN = "M"

#Converts ratings to indexes corresponding to "High", "Medium", and "Low"
def convert_rating_to_index(rating):
    if rating > 3.5:
        return 2
    if rating < 2.5:
        return 0
    return 1

#Updates the word_data dictionary to log an occurence of the specified word, whether it was for
#M or F, and whether it was high or low
def add_data_for_word(word_data, word, gender, rating):
    """
    This makes sure the word has a dictionary entry with values everyone so there aren't errors calling
    on empty entries later.
    """
    if not word in word_data:
        word_data[word] = {
            'W' : [0, 0, 0],
            'M' : [0, 0, 0]
        }
    index = convert_rating_to_index(rating)
    word_data[word][gender][index] += 1

#Reads and cleans the file, then adds each word into the word_data dictionary
def read_file(filename):
    word_data = {}
    data_file = open(filename)
    next(data_file)
    lines = data_file.readlines()
    for each in lines:
        clean_line = each.split(',')
        rating = float(clean_line[0])
        gender = clean_line[1]
        word_list = clean_line[2].split()
        for word in word_list:
            add_data_for_word(word_data, word, gender, rating)
    return word_data

#Returns all instances of a word in the word_data dictionary
def search_words(word_data, target):
    matches = []
    lower_target = target.lower()
    for key in word_data:
        lower_key = key.lower()
        if lower_target in lower_key:
            matches.append(key)
    return matches

#Testing function; prints every word in the word_data dictionary
def print_words(word_data):
    for key, value in sorted(word_data.items()):
        print(key, end=" ")
        for gender, counts in sorted(value.items()):
            print(gender, counts, end=" ")
        print("")

#This function was provided
def main():
    import sys
    args = sys.argv[1:]

    if len(args) == 0:
        return
    # Two command line forms
    # 1. data_file
    # 2. -search target data_file

    # Assume no search, so filename to read
    # is the first argument
    filename = args[0]

    # Check if we are doing search, set target variable
    target = ''
    if len(args) >= 2 and args[0] == '-search':
        target = args[1]
        filename = args[2]  # Update filename to skip first 2 args

    # Read in the data from the file name
    word_data = read_file(filename)

    # Either we do a search or just print everything.
    if len(target) > 0:
        search_results = search_words(word_data, target)
        for word in search_results:
            print(word)
    else:
        print_words(word_data)


if __name__ == '__main__':
    main()
