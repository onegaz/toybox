import java.util.*;

class CompositeValue {
	int data;
	String text;
	public CompositeValue(int data, String text) {
		this.data = data;
		this.text = text;
	}
	public Integer getKey() {
		return Integer.valueOf(data);
	}
	public String getValue() {
		return text;
	}
}

class ComparableCompositeValue extends CompositeValue 
	implements Comparable<ComparableCompositeValue> {

	public ComparableCompositeValue(int data, String text) {
		super(data, text);
	}

	@Override
	public int compareTo(ComparableCompositeValue o) {
		if (data < o.getKey()) 
			return -1;
		if (data > o.getKey())
			return 1;
		return text.compareTo(o.getValue());
	}
	
}

public class MapDemo {

	public static void main(String[] args) {
		System.out.println(System.getProperty("java.home"));
		System.out.printf("java version is %s\n", System.getProperty("java.version"));
		mapWithComparator();
		mapWithComparableCompositeValue();
		mapWithLambdaComparator();
	}
	
	static void mapWithComparableCompositeValue() {
		System.out.println("TreeMap with Comparable key");
		TreeMap<ComparableCompositeValue, String> map = new TreeMap<>();
		map.put(new ComparableCompositeValue(1000, "one"), "1000 one");
		map.put(new ComparableCompositeValue(9000, "one"), "9000 one");
		map.put(new ComparableCompositeValue(5000, "one"), "5000 one");
		map.put(new ComparableCompositeValue(2000, "one"), "2000 one");
		map.entrySet().forEach(entry -> System.out.println(entry.getValue()));
	}
	
	static void mapWithComparator() {
		System.out.println("TreeMap with Comparator");
		TreeMap<CompositeValue, String> map = new TreeMap<>(new Comparator<CompositeValue>() {
			public int compare(CompositeValue o1, CompositeValue o2) {
				if (o1.getKey() < o2.getKey()) return -1;
				if (o1.getKey() > o2.getKey()) return 1;
				return o1.getValue().compareTo(o2.getValue());
			}
		});
		map.put(new CompositeValue(1001, "one"), "1001 one");
		map.put(new CompositeValue(9001, "one"), "9001 one");
		map.put(new CompositeValue(5001, "one"), "5001 one");
		map.put(new CompositeValue(2001, "one"), "2001 one");
		map.entrySet().forEach(entry -> System.out.println(entry.getValue()));
	}

	static void mapWithLambdaComparator() {
		System.out.println("TreeMap with Lambda Comparator");
		TreeMap<CompositeValue, String> map = new TreeMap<>(
				(o1, o2) -> ((o1.getKey()!=o2.getKey())?o1.getKey().compareTo(o2.getKey()):
					o1.getValue().compareTo(o2.getValue())));
		map.put(new CompositeValue(100, "one"), "100 one");
		map.put(new CompositeValue(900, "one"), "900 one");
		map.put(new CompositeValue(500, "one"), "500 one");
		map.put(new CompositeValue(200, "one"), "200 one");
		for (Map.Entry<CompositeValue, String> entry : map.entrySet()) {
			System.out.println(entry.getValue());
		}
	}
}
