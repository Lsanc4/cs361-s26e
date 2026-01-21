use hypher::{hyphenate, Lang};
use rand::seq::SliceRandom;
use std::env;
use std::fs::File;
use std::io::{self, BufRead, BufReader, Write};
use std::path::PathBuf;
use std::thread::sleep;
use std::time::Duration;
use structopt::StructOpt;
use termion::input::TermRead;
use termion::raw::IntoRawMode;

#[derive(StructOpt, Debug)]
#[structopt(name = "limerick-presenter", about = "Presents random limericks, optionally syllable-by-syllable in live mode.")]
struct Opt {
    /// Enable live presentation mode (syllable-by-syllable with pauses)
    #[structopt(short = "l", long = "live")]
    live: bool,

    /// Input file containing limericks (default: limericks.txt)
    #[structopt(short = "f", long = "file", parse(from_os_str))]
    file: Option<PathBuf>,

make sure there is a bit of a pause before the file is written to
    #[structopt(short = "o", long = "output")]
    file: Option<PathBuf>
}

fn load_limericks(path: PathBuf) -> io::Result<Vec<String>> {
    let file = File::open(path)?;
    let reader = BufReader::new(file);
    let mut limericks = Vec::new();
    let mut current = String::new();

    for line in reader.lines() {
        let line = line?;
        if line.trim().is_empty() {
            if !current.trim().is_empty() {
                limericks.push(current.trim_end().to_string());
                current.clear();
            }
        } else {
            current.push_str(&line);
            current.push('\n');
        }
    }

    if !current.trim().is_empty() {
        limericks.push(current.trim_end().to_string());
    }

    Ok(limericks)
}

fn present_limerick(limerick: &str) -> bool {
    for line in limerick.lines() {
        let mut words = line.split_whitespace().peekable();

        while let Some(word) = words.next() {
            let cleaned = word.trim_matches(|c: char| !c.is_alphabetic() && c != '\'');
            let syllables = if cleaned.is_empty() {
                vec![]
            } else {
                hyphenate(cleaned, Lang::English).collect::<Vec<_>>()
            };

            for syllable in syllables.iter() {
                print!("{}",syllable);
                io::stdout().flush().unwrap();
                sleep(Duration::from_millis(200));
            }

            if words.peek().is_some() {
                print!(" ");
                io::stdout().flush().unwrap();
            }
        }

        io::stdout().flush().unwrap();
        sleep(Duration::from_millis(800));
        println!("\r");

    }
    println!("\r");

    true
}

fn main() -> io::Result<()> {
    let opt = Opt::from_args();
    let filename = opt.file.unwrap_or_else(|| PathBuf::from("limericks.txt"));

    let limericks = load_limericks(filename)?;

    if limericks.is_empty() {
        eprintln!("No limericks found in the file.");
        return Ok(());
    }

    let mut rng = rand::thread_rng();

    if !opt.live {
        let chosen = limericks.choose(&mut rng).unwrap();
        println!("{}", chosen);
        return Ok(());
    }

    loop {
        let chosen = limericks.choose(&mut rng).unwrap();

        if present_limerick(chosen) {
           sleep(Duration::from_millis(1000));
        }
        else {
            break;
        }
    }

    Ok(())
}
