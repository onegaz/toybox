import java.io.*;
import java.util.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.CharBuffer;
import java.awt.Toolkit;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.FileDialog;
import java.awt.Container;
import javax.swing.*;

public class csvmerge {
	public static void addComponentsToPane(JFrame frame) {
		Container pane=frame.getContentPane();
		pane.setLayout(new GridBagLayout());
		GridBagConstraints c = new GridBagConstraints();
	
		JButton button = new JButton("Base CSV");
		c.fill = GridBagConstraints.NONE;
		c.gridx = 0;
		c.gridy = 0;
		c.weightx = 0;
		pane.add(button, c);

		JTextField baseCsvEdit = new JTextField(12);
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridx = 1;
		pane.add(baseCsvEdit, c);
		
		button.addActionListener((event) -> {
			FileDialog fd = new FileDialog(frame, "Choose base csv file", FileDialog.LOAD);
			fd.setFile("*.csv");
			fd.setVisible(true);
			String filename = fd.getFile();
			if (filename != null)
				baseCsvEdit.setText(fd.getDirectory() + filename);
			}
		);

		button = new JButton("CSV");
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.gridx = 0;
		c.gridy++;
		pane.add(button, c);
	
		JTextField extraCsvEdit = new JTextField(12);
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridx = 1;
		
		pane.add(extraCsvEdit, c);
		button.addActionListener((event) -> {
			FileDialog fd = new FileDialog(frame, "Choose another csv file", FileDialog.LOAD);
			fd.setFile("*.csv");
			fd.setVisible(true);
			String filename = fd.getFile();
			if (filename != null)
				extraCsvEdit.setText(fd.getDirectory() + filename);
			}
		);
	
		JLabel label = new JLabel("Key Column");
		c.fill = GridBagConstraints.NONE;
		c.gridx = 0;    
		c.gridy++;       
		c.weightx = 0;
		pane.add(label, c);
		
		JTextField keyColumnEdit = new JTextField(12);
		c.gridx++;
		c.weightx = 1;
		c.anchor = (c.gridx != 0) ? GridBagConstraints.WEST : GridBagConstraints.EAST;
		pane.add(keyColumnEdit, c);
		
		button = new JButton("Output");
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.gridx = 0;
		c.gridy++;
		pane.add(button, c);
	
		JTextField outputCsvEdit = new JTextField(12);
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridx = 1;
		
		pane.add(outputCsvEdit, c);
		button.addActionListener((event) -> {
		      JFileChooser fc = new JFileChooser();
		      int rVal = fc.showSaveDialog(frame);
		      if (rVal == JFileChooser.APPROVE_OPTION) {
		        outputCsvEdit.setText(fc.getSelectedFile().getAbsolutePath());
		      }
		});
		
		button = new JButton("Merge");
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 0;
		c.gridx = 0;
		c.gridwidth = 2;
		c.gridy++;
		pane.add(button, c);
		button.addActionListener((event) -> {
			String base = baseCsvEdit.getText();
			String extra = extraCsvEdit.getText();
			String output = outputCsvEdit.getText();
			String columnKeyStr = keyColumnEdit.getText();
			if (base!=null && extra!=null && output!=null && columnKeyStr!=null) {
				String[] args = new String[4];
				args[0] = base;
				args[1] = extra;
				args[2] = output;
				args[3] = columnKeyStr;
				mergeCsvFiles(args);
			}
		});
		
    }
	
	private static void createAndShowGUI() {
		JFrame frame = new JFrame("GridBagLayoutDemo");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();

		int height = 180;
		int width = screenSize.width * 2 / 3;
		frame.setPreferredSize(new Dimension(width, height));
		addComponentsToPane(frame);
		frame.setSize(new Dimension(width, height));
		frame.setLocationRelativeTo(null);
		frame.setVisible(true);
	}
    
	static HashMap<String, String> csvToMap(String filepath, int columnkey) {
		HashMap<String, String> hmap = new HashMap<String, String>();
		try {
			BufferedReader reader = new BufferedReader(new FileReader(filepath));
			String line = reader.readLine();
			while (line != null) {
				String[] splitcols = line.split(",", columnkey + 2);
				hmap.put(splitcols[columnkey], line);
				line = reader.readLine();
			}
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return hmap;
	}
	
	static List<String> csvToList(String filepath, int columnkey) {
		List<String> hmap = new ArrayList<>();
		try {
			BufferedReader reader = new BufferedReader(new FileReader(filepath));
			String line = reader.readLine();
			while (line != null) {
				String[] splitcols = line.split(",", columnkey + 2);
				hmap.add(splitcols[columnkey]);
				line = reader.readLine();
			}
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return hmap;
	}	
	
	static String maybeAppendComma(String line, int cnt) {
		int commacnt = getCommaCount(line);
		if (commacnt == cnt)
			return line;
		StringBuffer sb = new StringBuffer();
		sb.append(line);
		while (commacnt<cnt) {
			//line += ",";
			sb.append(',');
			commacnt++;
		}
		return sb.toString();
	}
	
	static int getCommaCount(String strCSVRecord) {
		// ignore comma in double quotes
		boolean quoted = false;
		int commaCnt = 0;
		for(char c : strCSVRecord.toCharArray()){
			switch(c) {
			case ',':
				if (!quoted)
					commaCnt++;
				break;
			case '\"':
			case '\'':
				quoted = !quoted;
				break;
			default:
				break;
			}
		}
		return commaCnt;
	}
	
	static int getColumnCount(String strCSVRecord) {
		// ignore comma in double quotes
		String[] strParts = strCSVRecord.split(",(?=([^\"]*\"[^\"]*\")*[^\"]*$)", -1);
		return strParts.length;
	}
	
	static void mergeCsvFiles(String[] args) {
		if (args.length < 4) {
			System.out.println("Usage: java csvmerge <base csv> <extra csv> <out path> <column num>");
			return;
		}
		String outputpath = args[2];
		int columnkey = Integer.valueOf(args[3]);
		HashMap<String, String> hmap = csvToMap(args[1], columnkey);
		
		File file = new File(outputpath);
		if(file.exists() && !file.isDirectory()) { 
			System.out.printf("output file already exist %s" + System.getProperty("line.separator"), outputpath);
			//return;
		}
		HashMap<String, String> basecsv = csvToMap(args[0], columnkey);
		
        FileWriter fr = null;
        int lines_only_in_base = 0;
        int lines_not_in_base = 0;
        int lines_in_both = 0;
        int column_count = 0;
        try {
            fr = new FileWriter(file);
            BufferedReader reader = new BufferedReader(new FileReader(args[0]));
			String line = reader.readLine();
			while (line != null) {
				String[] splitcols = line.split(",", columnkey+2);
				// System.out.println(splitcols[columnkey]);
				// System.out.println(splitcols[columnkey+1]);
				if (column_count<1)
					column_count = getColumnCount(line); // find column count from first line
				//column_count = Math.max(column_count, getColumnCount(line));
				
				if (hmap.containsKey(splitcols[columnkey])) {
					lines_in_both++;
					fr.write(maybeAppendComma(line, column_count) + hmap.get(splitcols[columnkey])
							+ System.getProperty("line.separator"));
				}
				else {
					lines_only_in_base++;
					fr.write(line + CharBuffer.allocate(column_count).toString().replace( '\0', ',' )
							+ System.getProperty("line.separator"));
				}
				line = reader.readLine();
			}
			reader.close();
			
			// for lines only exist in second file
			StringBuffer sb = new StringBuffer();
			for(int i=0; i<column_count; i++)
				sb.append(',');
			String basePlaceHolder = sb.toString(); // CharBuffer.allocate(column_count).toString().replace('\0', ',')
			
			List<String> secondcsv = csvToList(args[1], columnkey);
			for (String key: secondcsv) {
				if (!basecsv.containsKey(key)) {
					lines_not_in_base++;
					line = hmap.get(key);
					fr.write(basePlaceHolder + line
							+ System.getProperty("line.separator"));
					System.out.println("Line not in base: " + line);
				}				
			}
			
//			for (Map.Entry<String, String> entry : hmap.entrySet()) {
//				String key = entry.getKey();
//				if (!basecsv.containsKey(key)) {
//					lines_not_in_base++;
//					line = entry.getValue();
//					fr.write(CharBuffer.allocate(column_count).toString().replace('\0', ',') + line
//							+ System.getProperty("line.separator"));
//					System.out.println("Line not in base: " + line);
//				}
//			}
			
        } catch (IOException e) {
            e.printStackTrace();
        }finally{
            //close resources
            try {
                fr.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        System.out.printf("Merged %d lines, %d line only in base, %d lines are not in base, output: %s" + System.getProperty("line.separator")
		, lines_in_both, lines_only_in_base, lines_not_in_base, outputpath);
	}
	
	public static void main(String[] args) {
		javax.swing.SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				createAndShowGUI();
			}
		});
		return;
	}
}